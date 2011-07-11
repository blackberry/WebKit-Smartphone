#!/bin/bash
#
# Copyright (C) Research In Motion Limited 2010. All rights reserved.
#
# Script that can be used to find out what version of webkit the release branches are currently using.
#
for v in 9 10 11 12; do
  echo "GCXR${v}";
  cl=$(p4 changes -m 1 //depot/projects/JavaDevice/2010.1-GCXR${v}/ExtVersions/OlympiaVersion.txt | awk '{print $2}')
  sha=$(p4 describe $cl | grep '> OLYMPIA_SHA' | awk -F',' '{print $2}')
  echo "WebKit SHA: $sha"
  git show $sha --format=format:"%an : %cD" | head -1
  echo -n "WebKit Branch:"
  git branch -r --contains $sha;
done
