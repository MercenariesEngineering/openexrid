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
for x in os.listdir("build/release/lib"):
	if re.match("nuke[0-9.]+", x):
		nuke_versions.append (x)

cwd = os.getcwd()
tmpdir = tempfile.mkdtemp ()
os.chdir (tmpdir)
zip_filename = cwd + "/openexrid-" + version + "-" + zipfile_platform
print ("Archiving in "+tmpdir+", output file is "+zip_filename)

try:
	os.mkdir ("openexrid")

	if os.path.exists(cwd + "/build/release/lib/openexrid.ofx"):
		os.makedirs ("openexrid/"+ofx_dst)
		shutil.copy (cwd + "/build/release/lib/openexrid.ofx", "openexrid/"+ofx_dst)
		strip ("openexrid/" + ofx_dst + "/openexrid.ofx")
	else:
		if os.path.exists(cwd + "/build/release/lib/OFX/openexrid.ofx"):
			os.makedirs ("openexrid/OFX/"+ofx_dst)
			shutil.copy (cwd + "/build/release/lib/OFX/openexrid.ofx", "openexrid/OFX/"+ofx_dst)
			strip ("openexrid/OFX/" + ofx_dst + "/openexrid.ofx")
		if os.path.exists(cwd + "/build/release/lib/OFX_nuke9/openexrid.ofx"):
			os.makedirs ("openexrid/OFX_nuke9/"+ofx_dst)
			shutil.copy (cwd + "/build/release/lib/OFX_nuke9/openexrid.ofx", "openexrid/OFX_nuke9/"+ofx_dst)
			strip ("openexrid/OFX_nuke9/" + ofx_dst + "/openexrid.ofx")

	shutil.copy (cwd + "/LICENSE", "openexrid/")

	for nuke in nuke_versions:
		if os.path.exists(cwd + "/build/release/lib/" + nuke + "/DeepOpenEXRId" + so_ext):
			os.mkdir ("openexrid/"+nuke)
			shutil.copy (cwd + "/build/release/lib/" + nuke + "/DeepOpenEXRId" + so_ext, "openexrid/"+nuke)
			strip ("openexrid/"+nuke + "/DeepOpenEXRId" + so_ext)
			shutil.copy (cwd + "/nuke/menu.py", "openexrid/"+nuke)

	shutil.make_archive (zip_filename, archive_type, ".", "openexrid")

finally:
	os.chdir (cwd)
	shutil.rmtree (tmpdir)
