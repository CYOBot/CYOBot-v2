import os
from lib.wireless import WiFi
import json
import urequests
import struct
import zlib

ZIP_PATH  = "/sdcard/portal.zip"
PORTAL_DIR = "/sdcard/portal"

def is_dir(path):
    return (os.stat(path)[0] & 0x4000) == 0x4000

def rmrf(path):
    for name in os.listdir(path):
        full = path + "/" + name
        if is_dir(full):
            rmrf(full)
            os.rmdir(full)
        else:
            os.remove(full)

def mkdir_p(path):
    parts = path.split("/")
    cur = ""
    for p in parts:
        if not p: continue
        cur += "/" + p
        try:
            os.mkdir(cur)
        except OSError:
            pass

def clean_after_update():
    os.remove(ZIP_PATH)

def connect_wifi():
    if "wifi" not in globals().keys():
        wifi = WiFi()
    if not wifi.wlan.isconnected():
        try:
            with open("/sdcard/config/robot-config.json") as file:
                content = json.loads(file.read())
            if content["wifi"]["ssid"] != "":
                print("Connecting to WiFi:", content["wifi"]["ssid"], content["wifi"]["password"])
                wifi.connect(content["wifi"]["ssid"], content["wifi"]["password"], verbose=True)
        except Exception as e:
            print("WiFi check and connect error:", e)
            pass

def download_update(URL):
    resp = urequests.get(URL)
    with open(ZIP_PATH, "wb") as f:
        while True:
            chunk = resp.raw.read(512)
            if not chunk:
                break
            f.write(chunk)
    resp.close()

    try:
        os.mkdir(PORTAL_DIR)
    except OSError:
        pass

    rmrf(PORTAL_DIR)

def unzip_update():
    # ensure destination root exists
    mkdir_p(PORTAL_DIR)

    with open(ZIP_PATH, "rb") as f:
        # jump to end, read last 1KiB (should contain the EOCD)
        f.seek(-1024, 2)
        tail = f.read(1024)
        idx = tail.rfind(b"PK\x05\x06")    # EOCD signature
        if idx < 0:
            raise ValueError("EOCD not found")
        eocd = tail[idx:idx+22]
        # unpack: 4s, disk, disk_start, nrec_disk, nrec, dir_size, dir_off, comment_len
        _, disk, disk_start, nrec_disk, nrec, dir_size, dir_off, _ = \
            struct.unpack("<4sHHHHIIH", eocd)

        # read central directory
        f.seek(dir_off)
        cdir = f.read(dir_size)

    offset = 0
    entries = []
    while offset < len(cdir):
        sig = cdir[offset:offset+4]
        if sig != b"PK\x01\x02":
            break  # done
        # central dir header is 46 bytes fixed
        header = cdir[offset:offset+46]
        (sig, ver_made, ver_need, flags, comp_method,
        mod_time, mod_date,
        crc32_c, comp_size, uncomp_size,
        fname_len, extra_len, comment_len,
        disk_start, int_attr,
        ext_attr, local_off) = struct.unpack("<4sHHHHHHIIIHHHHHII", header)

        # extract filename
        start = offset + 46
        name = cdir[start:start+fname_len].decode("utf-8").lstrip("./\\")
        entries.append({
            "name": name,
            "comp_method": comp_method,
            "comp_size": comp_size,
            "uncomp_size": uncomp_size,
            "local_offset": local_off
        })
        # move to next entry
        offset = start + fname_len + extra_len + comment_len

    with open(ZIP_PATH, "rb") as f:
        for e in entries:
            dest = PORTAL_DIR + "/" + e["name"]
            if e["name"].endswith("/"):
                # directory
                mkdir_p(dest)
                continue

            # ensure parent dirs
            parent = dest.rsplit("/", 1)[0]
            mkdir_p(parent)

            # seek to local header
            f.seek(e["local_offset"])
            sig = f.read(4)
            if sig != b"PK\x03\x04":
                raise ValueError("Bad local header at %d" % e["local_offset"])
            # skip fixed 26-byte part
            hdr = f.read(26)
            # unpack to get filename & extra lengths
            _, flags, _, _, _, _, _, _, fname_len_l, extra_len_l = \
                struct.unpack("<HHHHHIIIHH", hdr)
            # skip name & extra
            f.read(fname_len_l + extra_len_l)

            # read compressed data
            comp_data = f.read(e["comp_size"])

            # decompress or store
            if e["comp_method"] == 0:
                data = comp_data
            elif e["comp_method"] == 8:
                # raw DEFLATE (no wrapper)
                data = zlib.decompress(comp_data, -15)
            else:
                raise NotImplementedError(
                    "Unsupported compression %d in %s" 
                    % (e["comp_method"], e["name"])
                )

            # write file
            with open(dest, "wb") as out:
                out.write(data)

    print("Unzip complete!")