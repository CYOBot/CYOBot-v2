from lib.routine import ota

def ota_update_test(URL):
    ota.download_update(URL)
    ota.unzip_update()
    ota.clean_after_update()

URL = "https://quest.cyobot.com/api/media/file/python-portal.zip"
ota_update_test(URL)