"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
////////////////////////////////////////////////////////////////
var moov_to_start_1 = require("./moov_to_start");
if (process.argv.length != 4) {
    console.log("Usage " + process.argv[1] + " <in_file> <out_file>");
    process.exit(-1);
}
var in_file = process.argv[2];
var out_file = process.argv[3];
var mp4Util = new moov_to_start_1.MoveMoovAtom(in_file, out_file);
console.log(mp4Util);
mp4Util.moveMoovToStart();
