from pathlib import Path
import hashlib, json, urllib.request, zipfile
root = Path(__file__).resolve().parent
out = root / "tools"
out.mkdir(exist_ok=True)
for item in json.loads((root / "dependencies.json").read_text()):
    archive = out / item["file"]
    if not archive.exists():
        urllib.request.urlretrieve(item["url"], archive)
    digest = hashlib.sha256(archive.read_bytes()).hexdigest()
    if digest != item["sha256"]:
        raise RuntimeError("Checksum mismatch: " + item["file"])
    with zipfile.ZipFile(archive) as package:
        package.extractall(out)
    print("Verified and extracted:", item["file"])
