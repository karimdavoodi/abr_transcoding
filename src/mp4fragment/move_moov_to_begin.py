import sys
import time
## TODO: we must change all index inside stco (or co64) header at moov 
##          new_index = current_index + moov_size // because only 'moov' moved before mdat
##  https://developer.apple.com/library/archive/documentation/QuickTime/QTFF/QTFFChap2/qtff2.html
def read_header(data, start = 0):
    if len(data) < 8 : 
        return 0, ""
    size = int.from_bytes(data[start:start+4], "big")
    htype = data[start+4:start+8]
    return size, htype

def move_moov_to_start(in_file, out_file):
    out_f = open(out_file, "wb")
    with open(in_file, "rb") as in_f:
        # Process ftyp
        b8 = in_f.read(8)
        size, m_type = read_header(b8)
        if m_type != b'ftyp': 
            print(f"Invalid ftyp in {in_file}")
            return
        print("Write ftyp to out_file")
        out_f.write(b8)
        out_f.write(in_f.read(size-8))
        # Search moov
        while True:
            b8 = in_f.read(8)
            if len(b8) != 8: break
            size, m_type = read_header(b8)
            if m_type == b'moov': break
            in_f.read(size - 8)
            print(f"jump from {m_type}")
        if m_type == b'moov': 
            print("Write moov to out_file")
            out_f.write(b8)
            out_f.write(in_f.read(size-8))
        else:
            print(f"Not found moov box")
            return
    # Write all box to out_file except 'ftyp' and 'moov'
    with open(in_file, "rb") as in_f:
        while True:
            b8 = in_f.read(8)
            if len(b8) != 8: break
            size, m_type = read_header(b8)
            if m_type != b'ftyp' and m_type != b'moov' and m_type != b'free': 
                print(f"Write {m_type} to out_file")
                out_f.write(b8)
                out_f.write(in_f.read(size-8))
            else:
                in_f.read(size-8)
        


if len(sys.argv) != 3:
    print(f"Usage: {sys.argv[0]} <in_file> <out_file>")
    sys.exit(0)
in_file = sys.argv[1]
out_file = sys.argv[2]
move_moov_to_start(in_file, out_file)


'''

    private function _swapIndex()
    {
        $moovSize = $this->_moovBytes->bytesAvailable();

        $moovAType = '';
        $moovASize = 0;
        $offsetCount = 0;

        $compressionCheck = $this->_moovBytes->readBytes(12, 4);

        if ($compressionCheck == Atom::CMOV_ATOM) {
            throw new Exception('compressed MP4/QT-file can\'t do this file: '.$file);
        }

        // begin of metadata
        $metaDataOffsets = array();
        $metaDataStrings = array();
        $metaDataCurrentLevel = 0;

        $moovStartOffset = 12;

        for ($i = $moovStartOffset; $i < $moovSize - $moovStartOffset; $i++) {
            $moovAType = $this->_moovBytes->readUTFBytes($i, 4);

            if (Atom::isValidAtom($moovAType)) {

                $moovASize = $this->_moovBytes->readUnsignedInt($i - 4);

                if (($moovASize > 8) && ($moovASize + $i < ($moovSize - $moovStartOffset))) {

                    try {
                        $containerLength = 0;
                        $containerString = $moovAType;

                        for ($mi = count($metaDataOffsets) - 1; $mi > - 1; $mi--) {

                            $containerLength = $metaDataOffsets[$mi];

                            if ($i - $moovStartOffset < $containerLength && $i - $moovStartOffset + $moovASize > $containerLength) {
                                throw new Exception('bad atom nested size');
                            }

                            if ($i - $moovStartOffset == $containerLength) {
                                array_pop($metaDataOffsets);
                                array_pop($metaDataStrings);
                            } else {
                                $containerString = $metaDataStrings[$mi].".".$containerString;
                            }
                        }

                        if (($i - $moovStartOffset) <= $containerLength) {
                            array_push($metaDataOffsets, ($i - $moovStartOffset + $moovASize));
                            array_push($metaDataStrings, $moovAType);
                        }

                        if ($moovAType != Atom::STCO_ATOM && $moovAType != Atom::CO64_ATOM) {
                            $i += 4;
                        } elseif ($moovAType == Atom::URL_ATOM || $moovAType == Atom::XML_ATOM) {
                            $i += $moovASize - 4;
                        }
                    }
                    catch(Exception $e) {
                        echo 'EXCEPTION: '.$e->getMessage();
                    }
                }
            }


            if ($moovAType == Atom::STCO_ATOM) {
                $moovASize = $this->_moovBytes->readUnsignedInt($i - 4);

                if ($i + $moovASize - $moovStartOffset > $moovSize) {
                    throw new Exception('bad atom size');
                    return;
                }

                $offsetCount = $this->_moovBytes->readUnsignedInt($i + 8);

                for ($j = 0; $j < $offsetCount; $j++) {
                    $position = ($i + 12 + $j * 4);

                    $currentOffset = $this->_moovBytes->readUnsignedInt($position);

                    // cause of mooving the moov-atom right before the rest of data
                    // (behind ftyp) the new offset is caluclated:
                    // current-offset + size of moov atom (box) = new offset
                    $currentOffset += $moovSize;

                    $this->_moovBytes->writeBytes(Transform::toUInt32BE($currentOffset), $position + 1);
                }
                $i += $moovASize - 4;

            } else if ($moovAType == Atom::CO64_ATOM) {
                $moovASize = $this->_moovBytes->readUnsignedInt($i - 4);

                if ($i + $moovASize - $moovStartOffset > $moovSize) {
                    throw new Exception('bad atom size');
                    return;
                }

                $offsetCount = $this->_moovBytes->readDouble($i + 8);

                for ($j2 = 0; $j2 < $offsetCount; $j2++) {
                    $position = ($i + 12 + $j * 8);

                    $currentOffset = $this->_moovBytes->readUnsignedInt($position);

                    // cause of mooving the moov-atom right before the rest of data
                    // (behind ftyp) the new offset is caluclated:
                    // current-offset + size of moov atom (box) = new offset
                    $currentOffset += $moovSize;

                    // TODO implement!
                    //$this->_moovBytes->writeBytes(Transform::toUInt64BE($currentOffset), $position+1);
                }
                $i += $moovASize - 4;
            }
        }

        return true;
    }


'''
