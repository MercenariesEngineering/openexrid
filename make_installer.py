import tempfile, shutil, os, re, platform, zipfile

def get_version ():
	file = open("openexrid/Version.h", "r")
	for line in file:
		version = re.search("Version\s*=\s*\"([^\"]+)", line)
		if version:
			return version.group(1)

def strip (path):
	if platform.system() == "Linux":
		print ("strip " + path)
		os.system ("strip " + path)

version = get_version ()
ofx_plateform = "Win64" if platform.system() == "Windows" else "Linux-x86-64"
ofx_dst = "openfx/openexrid.ofx.bundle/Contents/" + ofx_plateform
zipfile_platform = "win64" if platform.system() == "Windows" else "linux"
so_ext = ".dll" if platform.system() == "Windows" else ".so"
archive_type = "zip" if platform.system() == "Windows" else "gztar"

nuke_versions = []
for x in os.listdir("."):
	if re.match("nuke[0-9.]+", x):
		nuke_versions.append (x)

cwd = os.getcwd()
tmpdir = tempfile.mkdtemp ()
os.chdir (tmpdir)
zip_filename = cwd + "/openexrid-" + version + "-" + zipfile_platform

try:
	os.makedirs (ofx_dst)
	shutil.copy (cwd + "/openfx/release/openexrid.ofx", ofx_dst)
	strip (ofx_dst + "/openexrid.ofx")
	shutil.copy (cwd + "/LICENSE", ".")

	for nuke in nuke_versions:
		os.mkdir (nuke)
		shutil.copy (cwd + "/" + nuke + "/release/DeepOpenEXRId" + so_ext, nuke)
		strip (nuke + "/DeepOpenEXRId" + so_ext)
		shutil.copy (cwd + "/nuke/menu.py", nuke)

	shutil.make_archive (zip_filename, archive_type, tmpdir)

finally:
	os.chdir (cwd)
	shutil.rmtree (tmpdir)
