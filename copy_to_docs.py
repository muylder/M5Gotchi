import shutil
import os

def after_build(source, target, env):
    # Path to the built firmware file
    firmware_path = os.path.join(env.subst("$BUILD_DIR"), "firmware.bin")

    # Destination path (in the repo)
    dest_path = os.path.join(env["PROJECT_DIR"], "docs", "firmware", "latest.bin")

    # Ensure destination folder exists
    os.makedirs(os.path.dirname(dest_path), exist_ok=True)

    print(f"Copying firmware to {dest_path}")
    shutil.copyfile(firmware_path, dest_path)

# Register the callback
Import("env")
env.AddPostAction("buildprog", after_build)
