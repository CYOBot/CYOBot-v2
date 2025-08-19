from sh1106 import SH1106_I2C
import machine, time, math

# --- init ---
i2c = machine.I2C(scl=machine.Pin(18), sda=machine.Pin(17))
display = SH1106_I2C(128, 64, i2c, None, 0x3c, rotate=0, delay=0)

# --- drawing primitives ---
def fill_circle(cx, cy, r, color):
    "Draw a filled circle at (cx,cy) with radius r."
    for dy in range(-r, r + 1):
        dx = int((r * r - dy * dy) ** 0.5)
        display.hline(cx - dx, cy + dy, 2 * dx + 1, color)

def draw_eye(cx, cy, open_ratio, pupil_offset):
    """
    Draw one eye.
      cx,cy        center
      open_ratio   1.0=open, 0.0=closed
      pupil_offset horizontal pupil shift
    """
    R = 16  # eyeball radius
    # fully closed: just a line
    if open_ratio <= 0.0:
        display.line(cx - R, cy, cx + R, cy, 1)
        return

    # eyeball
    fill_circle(cx, cy, R, 1)
    # pupil (black disk)
    fill_circle(cx + pupil_offset, cy, R // 3, 0)

    # eyelid cover (black rects from top & bottom)
    total_cover = int((1 - open_ratio) * (2 * R))
    top_h = total_cover // 2
    bot_h = total_cover - top_h
    if top_h > 0:
        display.fill_rect(cx - R, cy - R, 2 * R, top_h, 0)
    if bot_h > 0:
        display.fill_rect(cx - R, cy + R - bot_h + 1, 2 * R, bot_h, 0)

# --- animation loop ---
blink_interval = 3000      # ms between blinks
last_blink = time.ticks_ms()

while True:
    now = time.ticks_ms()
    # pupil oscillates left-right
    pupil_x = int(4 * math.sin(now / 500))

    # trigger blink
    if time.ticks_diff(now, last_blink) > blink_interval:
        # blink closing
        for r in (1.0, 0.6, 0.3, 0.0):
            display.fill(0)
            draw_eye(40, 32, r, pupil_x)
            draw_eye(88, 32, r, pupil_x)
            display.show()
            time.sleep(0.05)
        # blink opening
        for r in (0.3, 0.6, 1.0):
            display.fill(0)
            draw_eye(40, 32, r, pupil_x)
            draw_eye(88, 32, r, pupil_x)
            display.show()
            time.sleep(0.05)

        last_blink = now
    else:
        # just eyes tracking
        display.fill(0)
        draw_eye(40, 32, 1.0, pupil_x)
        draw_eye(88, 32, 1.0, pupil_x)
        display.show()
        time.sleep(0.05)
