from __future__ import print_function
import argparse
import json
import os
import sys

def main(resources_path, dummy_resource_file_name=None):
  num_files_processed = 0
  resources = []
  for file_name in os.listdir(resources_path):
    file_base_name, file_extension = os.path.splitext(file_name)
    file_extension_without_dot = file_extension[1:]
    if file_extension_without_dot not in ['png']:
      continue

    # Replace spaces with underscores and capitalize
    resource_name = file_base_name.replace(' ', '_').upper()

    resources.append({
      'type': file_extension_without_dot,
      'name': resource_name,
      'file': file_name
    })
    num_files_processed += 1

  if dummy_resource_file_name:
    # Add a dummy resource whose ID we can use to know how many resources are installed
    resources.append({
      'type': 'raw',
      'name': 'NUM_RESOURCES',
      'file': dummy_resource_file_name
    })

  print(json.dumps(resources, indent=2, separators=(',', ': ')))
  print('Processed %d files' % num_files_processed, file=sys.stderr)

if __name__ == '__main__':
  parser = argparse.ArgumentParser(description='Process some integers.')
  parser.add_argument('resources_path', type=str, help='The path to the resources')
  parser.add_argument('--dummy_resource', type=str, help='Optional filename for a dummy resource')
  args = parser.parse_args()

  main(args.resources_path, args.dummy_resource)
