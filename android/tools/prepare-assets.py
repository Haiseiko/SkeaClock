#!/usr/bin/env python3
"""
prepare-assets.py

Usage:
  python3 prepare-assets.py --source-zip /path/to/source.zip --out-dir ../app/src/main/assets/processed

This script extracts a provided source.zip (which should contain the original "source" folder
from the desktop SkeaClock repo) and converts GIF assets to PNG frame sequences suitable for
including in the Android app's assets. It also copies JSON files used by the original
prepare-assets.ps1 into the output so the native loader can read layout/point data.

Requirements:
  - Python 3.7+
  - Pillow (pip install pillow)

The script is intentionally conservative: it doesn't try to fully reproduce the PS1 logic
but extracts GIF frames and copies JSON files. After running this, run the Android build
or commit the generated assets into the repo.
"""

import argparse
import zipfile
import os
import sys
from pathlib import Path
from PIL import Image, ImageSequence
import shutil
import json


def extract_zip(zip_path: Path, dest: Path):
    print(f"Extracting {zip_path} -> {dest}")
    with zipfile.ZipFile(zip_path, 'r') as z:
        z.extractall(dest)


def find_gifs(src: Path):
    return list(src.rglob('*.gif'))


def find_jsons(src: Path):
    return list(src.rglob('*.json'))


def ensure_dir(p: Path):
    p.mkdir(parents=True, exist_ok=True)


def extract_gif_frames(gif_path: Path, out_dir: Path):
    print(f"Processing GIF: {gif_path}")
    ensure_dir(out_dir)
    im = Image.open(gif_path)
    frame_index = 0
    durations = []
    for frame in ImageSequence.Iterator(im):
        frame = frame.convert('RGBA')
        out_file = out_dir / f"frame_{frame_index:04d}.png"
        frame.save(out_file)
        duration = frame.info.get('duration', 100)  # milliseconds
        durations.append(duration)
        frame_index += 1
    # save metadata (frame durations)
    meta = {
        'source': gif_path.name,
        'frameCount': frame_index,
        'durationsMs': durations
    }
    (out_dir / 'frames.json').write_text(json.dumps(meta, indent=2))
    print(f"  extracted {frame_index} frames to {out_dir}")


def copy_jsons(json_paths, out_dir: Path, src_root: Path):
    for p in json_paths:
        rel = p.relative_to(src_root)
        dest = out_dir / rel
        ensure_dir(dest.parent)
        shutil.copy2(p, dest)
        print(f"Copied JSON: {p} -> {dest}")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--source-zip', required=True, help='Path to a zip file containing the original source/ folder')
    parser.add_argument('--out-dir', default='android/app/src/main/assets/processed', help='Output directory for processed assets')
    parser.add_argument('--work-dir', default='android/tools/_work', help='Temporary work directory')
    args = parser.parse_args()

    zip_path = Path(args.source_zip)
    if not zip_path.exists():
        print('source zip not found:', zip_path)
        sys.exit(2)

    work_dir = Path(args.work_dir)
    if work_dir.exists():
        shutil.rmtree(work_dir)
    work_dir.mkdir(parents=True)

    extract_zip(zip_path, work_dir)

    # locate a directory named 'source' in the extracted tree
    extracted_source = None
    for p in work_dir.iterdir():
        if p.is_dir() and p.name.lower() == 'source':
            extracted_source = p
            break
    if extracted_source is None:
        # fallback: use entire work_dir as source
        extracted_source = work_dir

    out_root = Path(args.out_dir)
    ensure_dir(out_root)

    # process GIFs
    gifs = find_gifs(extracted_source)
    if not gifs:
        print('Warning: no GIFs found in source. Check your zip contents.')
    for gif in gifs:
        rel = gif.relative_to(extracted_source)
        out_dir = out_root / rel.with_suffix('')
        extract_gif_frames(gif, out_dir)

    # copy JSONs (locate data etc.)
    jsons = find_jsons(extracted_source)
    if jsons:
        copy_jsons(jsons, out_root, extracted_source)
    else:
        print('No JSON files found in source; prepare-assets.ps1 data will not be available.')

    print('Asset preparation complete. Commit', out_root, 'to android/app/src/main/assets if desired.')


if __name__ == '__main__':
    main()
