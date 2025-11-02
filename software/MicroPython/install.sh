# Color variables
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}Scanning for Silicon Labs CP210x USB to UART Bridge...${NC}"

# Find the CP210x device port automatically
PORT=""
for device in /dev/ttyUSB* /dev/ttyACM*; do
    if [ -e "$device" ]; then
        # Check if this is a Silicon Labs CP210x device
        if udevadm info --name="$device" 2>/dev/null | grep -q "ID_VENDOR=Silicon_Labs\|ID_VENDOR_ID=10c4"; then
            PORT="$device"
            echo -e "${GREEN}Found CP210x device at $PORT${NC}"
            break
        fi
    fi
done

# Fallback: if udevadm doesn't work, try checking dmesg
if [ -z "$PORT" ]; then
    PORT=$(dmesg | grep -i "cp210x.*attached to" | tail -1 | sed 's/.*attached to //' | awk '{print $1}')
    if [ -n "$PORT" ]; then
        PORT="/dev/$PORT"
        echo -e "${YELLOW}Found device via dmesg at $PORT${NC}"
    fi
fi

if [ -z "$PORT" ]; then
    echo -e "${RED}No Silicon Labs CP210x device found.${NC}"
    echo -e "${YELLOW}Please check that the device is connected and try again.${NC}"
    exit 1
fi

echo -e "${GREEN}Using port: $PORT${NC}"

# Check if we have permission to access the port
if [ ! -w "$PORT" ]; then
    echo -e "${YELLOW}Setting permissions for $PORT${NC}"
    sudo chmod 666 "$PORT"
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}Permissions updated successfully${NC}"
    else
        echo -e "${RED}Failed to update permissions. Please run: sudo chmod 666 $PORT${NC}"
        exit 1
    fi
fi

echo -e "${GREEN}Erase memory${NC}"
esptool.py --port "$PORT" erase_flash

echo -e "${GREEN}Start writing CYOBot OS to board${NC}"
esptool.py -p "$PORT" -b 460800 --before default_reset --after hard_reset --chip esp32s3 write_flash --flash_mode dio --flash_size 4MB --flash_freq 80m 0x0 bootloader.bin 0x8000 partition-table.bin 0x10000 micropython.bin

echo -e "${GREEN}Copy files to /pyboard${NC}"
cd pyboard
rshell -p "$PORT" -b 115200 rsync . /pyboard

echo -e "${GREEN}Done${NC}"
