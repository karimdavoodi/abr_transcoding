"use strict";
var __createBinding = (this && this.__createBinding) || (Object.create ? (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    Object.defineProperty(o, k2, { enumerable: true, get: function() { return m[k]; } });
}) : (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    o[k2] = m[k];
}));
var __setModuleDefault = (this && this.__setModuleDefault) || (Object.create ? (function(o, v) {
    Object.defineProperty(o, "default", { enumerable: true, value: v });
}) : function(o, v) {
    o["default"] = v;
});
var __importStar = (this && this.__importStar) || function (mod) {
    if (mod && mod.__esModule) return mod;
    var result = {};
    if (mod != null) for (var k in mod) if (k !== "default" && Object.prototype.hasOwnProperty.call(mod, k)) __createBinding(result, mod, k);
    __setModuleDefault(result, mod);
    return result;
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.MoveMoovAtom = void 0;
var fs = __importStar(require("fs"));
/**
 * The Atom class
 * @name: name of atom
 * @size: size of atom
 * @location: position of atom in file
 */
var Atom = /** @class */ (function () {
    function Atom(name, size, location) {
        this.name = name;
        this.size = size;
        this.location = location;
    }
    Atom.prototype.toString = function () {
        return "name:" + this.name + ", size:" + this.size + ", location: " + this.location;
    };
    return Atom;
}());
/**
 * The MoveMoovAtom class: move 'moov' atom to start of file
 * @in_file: input file
 * @out_file: output file
 *
 * moveMoovToStart : move 'moov' to start of output file
 * Usag Example:
 *      const mp4Util = new MoveMoovAtom(in_file, out_file);
 *      mp4Util.moveMoovToStart();
 *
 */
var MoveMoovAtom = /** @class */ (function () {
    function MoveMoovAtom(in_file, out_file) {
        this.in_file = in_file;
        this.out_file = out_file;
        this.atoms = this.allAtoms();
    }
    /**
     * Generate output file and move 'moov' to start of it
     * @returns True if successfull else False
     */
    MoveMoovAtom.prototype.moveMoovToStart = function () {
        if (!this.isValidMp4()) {
            console.warn("Envalid mp4. exit!");
            return false;
        }
        if (this.isMoovAtStart()) {
            console.warn("Moov is at start of file, do nothing.");
            return false;
        }
        console.log("Start to move moov...");
        var out_f = fs.openSync(this.out_file, 'w');
        var in_f = fs.openSync(this.in_file, 'r');
        // Copy ftyp
        console.log("Copy ftyp");
        var ftyp_size = this.atoms[0].size;
        var ftyp_buf = new DataView(new ArrayBuffer(ftyp_size));
        fs.readSync(in_f, ftyp_buf, 0, ftyp_size, 0);
        fs.writeSync(out_f, ftyp_buf, 0, ftyp_size);
        // Read and change chunk index in moov
        console.log("Copy moov");
        var moov = this.getMoovAtom();
        var moov_buf = new DataView(new ArrayBuffer(moov.size));
        fs.readSync(in_f, moov_buf, 0, moov.size, moov.location);
        this.changeChunkIndex(moov_buf);
        fs.writeSync(out_f, moov_buf, 0, moov.size);
        // Write other atoms to output
        for (var _i = 0, _a = this.atoms; _i < _a.length; _i++) {
            var atom = _a[_i];
            if (atom.name != 'ftyp' && atom.name != 'moov') {
                console.log("Copy", atom.name);
                this.copyFile(in_f, atom.location, atom.size, out_f);
            }
        }
        fs.closeSync(in_f);
        fs.closeSync(out_f);
        return true;
    };
    MoveMoovAtom.prototype.toString = function () {
        var atoms = '';
        for (var _i = 0, _a = this.atoms; _i < _a.length; _i++) {
            var atom = _a[_i];
            atoms = '\n' + atom.toString();
        }
        return atoms;
    };
    /**
     * List of Atoms in input file
     * @returns Array of {name, size} object for atoms
     */
    MoveMoovAtom.prototype.allAtoms = function () {
        var atoms = new Array();
        var fd = fs.openSync(this.in_file, "r");
        var bufview = new DataView(new ArrayBuffer(4));
        var n = 0;
        var size = 0;
        var position = 0;
        while (true) {
            n = fs.readSync(fd, bufview, 0, 4, position);
            if (n != 4)
                break;
            size = bufview.getUint32(0);
            n = fs.readSync(fd, bufview, 0, 4, position + 4);
            if (n != 4)
                break;
            var type = new Uint8ClampedArray(bufview.buffer);
            var type_name = new TextDecoder().decode(type);
            atoms.push(new Atom(type_name, size, position));
            position += size;
        }
        return atoms;
    };
    /**
     *
     * @returns moov atom from this.atoms list
     */
    MoveMoovAtom.prototype.getMoovAtom = function () {
        for (var _i = 0, _a = this.atoms; _i < _a.length; _i++) {
            var atom = _a[_i];
            if (atom.name == 'moov')
                return atom;
        }
        return new Atom('', 0, 0);
    };
    /**
     * Chack validation of file:
     *
     * @returns True if input file is valid mp4 format
     */
    MoveMoovAtom.prototype.isValidMp4 = function () {
        if (this.atoms.length < 3) {
            console.error("Input file has less than three atom.");
            return false;
        }
        if (this.atoms[0].name != 'ftyp') {
            console.error("First atom is not 'ftyp'.");
            return false;
        }
        var hasMdat = false;
        var hasMoov = false;
        for (var _i = 0, _a = this.atoms; _i < _a.length; _i++) {
            var atom = _a[_i];
            if (atom.name == 'moov')
                hasMoov = true;
            if (atom.name == 'mdat')
                hasMdat = true;
        }
        if (!hasMdat || !hasMoov) {
            console.error("Input file has not mdat or moov.");
            return false;
        }
        return true;
    };
    MoveMoovAtom.prototype.isMoovAtStart = function () {
        for (var i = 0; i < this.atoms.length; i++) {
            var element = this.atoms[i];
            if (element.name == 'moov' && i == 1)
                return true;
        }
        return false;
    };
    MoveMoovAtom.prototype.copyFile = function (in_f, position, size, out_f) {
        var buf = new DataView(new ArrayBuffer(4096));
        while (size > 0) {
            var len = (size > 4096) ? 4096 : size;
            var n = fs.readSync(in_f, buf, 0, len, position);
            if (n <= 0)
                break;
            fs.writeSync(out_f, buf, 0, n);
            size -= n;
            position += n;
        }
    };
    MoveMoovAtom.prototype.readAtom = function (data, pos) {
        if (pos === void 0) { pos = 0; }
        var size = data.getInt32(pos);
        var _name = new Uint8Array(data.buffer.slice(pos + 4, pos + 8));
        var name = new TextDecoder().decode(_name);
        //console.log(`read atom ${name}`);
        return new Atom(name, size, pos);
    };
    MoveMoovAtom.prototype.changeSTCO = function (data, atom) {
        console.log("Try to change indexes for " + atom);
        var moov = this.getMoovAtom();
        var stco_pos = atom.location;
        var offset_count = data.getUint32(stco_pos + 12);
        console.log("Offset count: " + offset_count);
        var cell_size = (atom.name == 'co64') ? 8 : 4;
        for (var i = 0; i < offset_count; i++) {
            var pos = stco_pos + 16 + (i * cell_size);
            var old_offset = data.getUint32(pos);
            var new_offset = old_offset + moov.size;
            data.setUint32(pos, new_offset);
            //console.log(`pos:${pos}, cur_offset: ${old_offset}, new_offset:${new_offset} `);
        }
    };
    MoveMoovAtom.prototype.changeChunkIndex = function (moov_data) {
        var moov_size = moov_data.byteLength;
        var atom_container = ['trak', 'mdia', 'minf', 'stbl'];
        var pos = 8;
        while (pos < moov_size - 8) {
            var atom = this.readAtom(moov_data, pos);
            if (atom.name == 'stco' || atom.name == 'co64') {
                this.changeSTCO(moov_data, atom);
            }
            if (atom_container.indexOf(atom.name) != -1) {
                pos += 8;
            }
            else {
                pos += atom.size;
            }
        }
        return true;
    };
    return MoveMoovAtom;
}());
exports.MoveMoovAtom = MoveMoovAtom;
