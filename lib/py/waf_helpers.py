import os
import shutil
import json
import glob

def copy_current_pbw(ctx):
    """
    After the pebble bundle is built, this copies the resulting .pbw into the
    releases/<face> directory, appending the version number from package.json.
    Any older .pbw versions are moved to a 'previous' subdirectory.
    """
    face = ctx.path.name

    src_pbw = os.path.join(ctx.path.abspath(), 'build', '{}.pbw'.format(face))
    if not os.path.exists(src_pbw):
        return
        
    releases_dir = os.path.join(ctx.path.parent.parent.abspath(), 'releases', face)
    if not os.path.isdir(releases_dir):
        os.makedirs(releases_dir)

    pkg_path = os.path.join(ctx.path.abspath(), 'package.json')
    if os.path.exists(pkg_path):
        with open(pkg_path, 'r') as pkg_f:
            pkg_data = json.load(pkg_f)
        version = pkg_data.get('version', 'unknown')
        
        dest_filename = '{}-{}.pbw'.format(face, version)
        dest_file = os.path.join(releases_dir, dest_filename)
        
        previous_dir = os.path.join(releases_dir, 'previous')
        for existing_pbw in glob.glob(os.path.join(releases_dir, '*.pbw')):
            if os.path.basename(existing_pbw) != dest_filename:
                if not os.path.isdir(previous_dir):
                    os.makedirs(previous_dir)
                shutil.move(existing_pbw, os.path.join(previous_dir, os.path.basename(existing_pbw)))
                
        shutil.copy2(src_pbw, dest_file)
