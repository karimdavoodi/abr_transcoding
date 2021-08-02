#!/bin/python3
import sys
import logging

import util
import pipeline
import abr_transcode

logging.basicConfig(level=logging.INFO)


util.get_parameters()

# for test /home/karim/Videos/The_Good_Bad_Ugly.mkv
util.args['ABR_INPUT_SOURCE'] = 'file:///home/karim/Videos/The_Good_Bad_Ugly.mkv'
util.args['ABR_INPUT_SOURCE'] = 'file:///home/karim/Videos/20_mpeg2.ts'
util.args['ABR_INPUT_SOURCE'] = 'file:///home/karim/Videos/20_v.h264' 
util.args['ABR_INPUT_SOURCE'] = 'file:///home/karim/Videos/20.mp4'

if not util.args.get('ABR_INPUT_SOURCE'):
    logging.error("Please set parameters!")
    sys.exit(-1)

logging.info(str(util.args)) 

abr_transcode.run(util.args)

sys.exit(-1 if pipeline.error else 0)
