////////////////////////////////////////////////////////////////
import { MoveMoovAtom } from "./moov_to_start";

if (process.argv.length != 4) {
    console.log(`Usage ${process.argv[1]} <in_file> <out_file>`);
    process.exit(-1);
}
const in_file = process.argv[2];
const out_file = process.argv[3];
const mp4Util = new MoveMoovAtom(in_file, out_file);
console.log(mp4Util);
mp4Util.moveMoovToStart();

