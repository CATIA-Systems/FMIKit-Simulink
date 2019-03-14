% Copyright (c) 2019 Dassault Systemes. All rights reserved.

function blkStruct = slblocks

blkStruct.Name        = 'FMI Kit';
blkStruct.OpenFcn     = 'FMIKit.initialize';

blkStruct.Browser(1).Library = 'FMIKit_blocks';
blkStruct.Browser(1).Name    = 'FMIKit';
