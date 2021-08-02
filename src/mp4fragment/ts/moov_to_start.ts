import * as fs from 'fs';

/**
 * The Atom class
 * @name: name of atom
 * @size: size of atom
 * @location: position of atom in file 
 */
class Atom {
    name: string;
    size: number;
    location: number;
    constructor(name: string, size: number, location: number) {
        this.name = name;
        this.size = size;
        this.location = location;
    }
    toString(): string {
        return `name:${this.name}, size:${this.size}, location: ${this.location}`;
    }
}
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
class MoveMoovAtom {
    private in_file: string;
    private out_file: string;
    private atoms: Array<Atom>;

    constructor(in_file: string, out_file: string) {
        this.in_file = in_file;
        this.out_file = out_file;
        this.atoms = this.allAtoms();
    }
    /**
     * Generate output file and move 'moov' to start of it
     * @returns True if successfull else False
     */
    public moveMoovToStart(): boolean {
        if (!this.isValidMp4()) {
            console.warn("Envalid mp4. exit!");
            return false;
        }
        if (this.isMoovAtStart()) {
            console.warn("Moov is at start of file, do nothing.");
            return false;
        }

        console.log("Start to move moov...");
        const out_f = fs.openSync(this.out_file, 'w');
        const in_f = fs.openSync(this.in_file, 'r');

        // Copy ftyp
        console.log("Copy ftyp");
        const ftyp_size = this.atoms[0].size;
        let ftyp_buf = new DataView(new ArrayBuffer(ftyp_size));
        fs.readSync(in_f, ftyp_buf, 0, ftyp_size, 0);
        fs.writeSync(out_f, ftyp_buf, 0, ftyp_size);

        // Read and change chunk index in moov
        console.log("Copy moov");
        const moov = this.getMoovAtom();
        let moov_buf = new DataView(new ArrayBuffer(moov.size));
        fs.readSync(in_f, moov_buf, 0, moov.size, moov.location);
        this.changeChunkIndex(moov_buf);
        fs.writeSync(out_f, moov_buf, 0, moov.size);

        // Write other atoms to output
        for (const atom of this.atoms) {
            if (atom.name != 'ftyp' && atom.name != 'moov') {
                console.log("Copy", atom.name);
                this.copyFile(in_f, atom.location, atom.size, out_f);
            }
        }

        fs.closeSync(in_f);
        fs.closeSync(out_f);

        return true;
    }
    public toString(): string {
        let atoms: string = '';
        for (const atom of this.atoms) {
            atoms = '\n' + atom.toString();
        }
        return atoms;
    }

    /**
     * List of Atoms in input file
     * @returns Array of {name, size} object for atoms
     */
    private allAtoms(): Array<Atom> {
        const atoms = new Array<Atom>();
        const fd = fs.openSync(this.in_file, "r");
        let bufview = new DataView(new ArrayBuffer(4));
        let n: number = 0;
        let size: number = 0;
        let position: number = 0;
        while (true) {
            n = fs.readSync(fd, bufview, 0, 4, position);
            if (n != 4) break;
            size = bufview.getUint32(0);
            n = fs.readSync(fd, bufview, 0, 4, position + 4);
            if (n != 4) break;
            const type = new Uint8ClampedArray(bufview.buffer);
            const type_name = new TextDecoder().decode(type);
            atoms.push(new Atom(type_name, size, position));
            position += size;
        }
        return atoms;
    }
    /**
     * 
     * @returns moov atom from this.atoms list
     */
    private getMoovAtom(): Atom {
        for (const atom of this.atoms) {
            if (atom.name == 'moov') return atom;
        }
        return new Atom('', 0, 0);
    }
    /**
     * Chack validation of file:
     *  
     * @returns True if input file is valid mp4 format
     */
    private isValidMp4(): boolean {
        if (this.atoms.length < 3) {
            console.error("Input file has less than three atom.");
            return false;
        }
        if (this.atoms[0].name != 'ftyp') {
            console.error("First atom is not 'ftyp'.");
            return false;
        }
        let hasMdat: boolean = false;
        let hasMoov: boolean = false;
        for (const atom of this.atoms) {
            if (atom.name == 'moov') hasMoov = true;
            if (atom.name == 'mdat') hasMdat = true;
        }
        if (!hasMdat || !hasMoov) {
            console.error("Input file has not mdat or moov.");
            return false;
        }
        return true;
    }
    private isMoovAtStart(): boolean {
        for (let i = 0; i < this.atoms.length; i++) {
            const element = this.atoms[i];
            if (element.name == 'moov' && i == 1) return true;
        }
        return false;
    }
    private copyFile(in_f: number, position: number, size: number, out_f: number) {
        let buf = new DataView(new ArrayBuffer(4096));
        while (size > 0) {
            let len = (size > 4096) ? 4096 : size;
            let n = fs.readSync(in_f, buf, 0, len, position);
            if (n <= 0) break;
            fs.writeSync(out_f, buf, 0, n);
            size -= n;
            position += n;
        }
    }
    private readAtom(data: DataView, pos: number = 0): Atom {
        let size = data.getInt32(pos);
        const _name = new Uint8Array(data.buffer.slice(pos + 4, pos + 8));
        const name = new TextDecoder().decode(_name);
        //console.log(`read atom ${name}`);
        return new Atom(name, size, pos);
    }
    private changeSTCO(data: DataView, atom: Atom) {
        console.log(`Try to change indexes for ${atom}`);
        const moov = this.getMoovAtom();
        const stco_pos = atom.location;
        let offset_count = data.getUint32(stco_pos + 12);
        console.log(`Offset count: ${offset_count}`);
        const cell_size = (atom.name == 'co64') ? 8 : 4;  
        for (let i = 0; i < offset_count; i++) {
            let pos = stco_pos + 16 + (i * cell_size);
            let old_offset = data.getUint32(pos);
            let new_offset = old_offset + moov.size;
            data.setUint32(pos, new_offset);
            //console.log(`pos:${pos}, cur_offset: ${old_offset}, new_offset:${new_offset} `);
        }
    }
    private changeChunkIndex(moov_data: DataView): boolean {
        let moov_size: number = moov_data.byteLength;
        const atom_container: string[] = ['trak', 'mdia', 'minf', 'stbl'];
        let pos = 8;
        while (pos < moov_size - 8) {
            let atom = this.readAtom(moov_data, pos);
            if (atom.name == 'stco' || atom.name == 'co64') {
                this.changeSTCO(moov_data, atom);
            }
            if (atom_container.indexOf(atom.name) != -1) {
                pos += 8;
            } else {
                pos += atom.size;
            }
        }
        return true;
    }
}

export { MoveMoovAtom };