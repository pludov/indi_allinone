from os.path import join, isfile

Import("env")

FRAMEWORK_DIR = env.PioPlatform().get_package_dir("framework-arduinopico")
patchflag_path = join(FRAMEWORK_DIR, ".patching-done")

# patch file only if we didn't do it before
if not isfile(join(FRAMEWORK_DIR, ".patching-done")):
    original_file = join(FRAMEWORK_DIR, "lib", "memmap_default.ld")
    patched_file = join("patches", "copy_to_ram.patch")

    assert isfile(original_file) and isfile(patched_file)
    # Check error code and abort if patching failed
    ecode= env.Execute("patch %s %s" % (original_file, patched_file))
    if ecode:
        print("Patching failed!")
        exit(ecode)
    ecode = env.Execute("touch " + patchflag_path)
    if ecode:
        print("Creating patch flag failed!")
        exit(ecode)


    def _touch(path):
        with open(path, "w") as fp:
            fp.write("")

    env.Execute(lambda *args, **kwargs: _touch(patchflag_path))
