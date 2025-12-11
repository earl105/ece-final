import serial
import subprocess

COM_PORT = "COM5"   # CHANGE THIS
BAUD = 115200

# Current selected app group (1, 2, or 3)
current_group = None

# Base paths for your .exe files
# Make sure the folder path ends with a backslash (escaped as \\)
BASE_PATH = r"C:\Users\dearl\OneDrive\Documents\Arduino\final_ece\AppVolumeControl\\"


def launch_exe(exe_name):
    """Launches an exe from your base folder."""
    try:
        full_path = BASE_PATH + exe_name
        subprocess.Popen([full_path])
        print(f"Launched: {full_path}")
    except Exception as e:
        print(f"Failed to launch {exe_name}: {e}")


def handle_group_select(cmd):
    global current_group

    if cmd == "GROUP_1":
        current_group = "browsers"
        print("Selected Group 1 → Browsers")

    elif cmd == "GROUP_2":
        current_group = "meetings"
        print("Selected Group 2 → Meetings")

    elif cmd == "GROUP_3":
        current_group = "music"
        print("Selected Group 3 → Music")

    else:
        print("Invalid group command")


def handle_volume(cmd):
    if current_group is None:
        print("No group selected yet — ignoring volume command")
        return

    if cmd == "VOL_UP":
        exe = f"{current_group}+5.exe"
    elif cmd == "VOL_DOWN":
        exe = f"{current_group}-5.exe"
    else:
        return

    launch_exe(exe)


# ---------------------------
# Main serial loop
# ---------------------------

ser = serial.Serial(COM_PORT, BAUD, timeout=1)
print(f"Listening on {COM_PORT}...")

while True:
    line = ser.readline().decode(errors="ignore").strip()
    if not line:
        continue

    print(f"Received: {line}")

    if line.startswith("GROUP_"):
        handle_group_select(line)

    elif line in ("VOL_UP", "VOL_DOWN"):
        handle_volume(line)
