#include <stdio.h>
#include <stdlib.h>

#include "Ap4.h"
#include "Ap4ByteStream.h"
#include "Ap4MoovAtom.h"
#include "Ap4MovieFragment.h"

int main(int argc, char** argv)
{
    AP4_ByteStream* input = NULL;
    
    if(argc != 2){
        printf("Usage: %s <media_file>\n", argv[0]);
        return -1;
    }
    AP4_Result result = AP4_FileByteStream::Create(argv[1], 
                                AP4_FileByteStream::STREAM_MODE_READ, input);
    if (AP4_FAILED(result)) {
        fprintf(stderr, "ERROR: cannot open input (%d)\n", result);
        return -1;
    }
    //AP4_ByteStream* output = NULL;
    //AP4_FileByteStream::Create("-stdout", AP4_FileByteStream::STREAM_MODE_WRITE, output);
    AP4_DataBuffer *buff = new AP4_DataBuffer();
    AP4_MemoryByteStream* output = new AP4_MemoryByteStream(buff);

    AP4_AtomInspector* inspector = NULL;
    inspector = new AP4_JsonInspector(*output);
    inspector->SetVerbosity(0);

    AP4_Atom* atom;
    
    AP4_DefaultAtomFactory atom_factory;
    while (atom_factory.CreateAtomFromStream(*input, atom) == AP4_SUCCESS) {
        // remember the current stream position because the Inspect method
        // may read from the stream (there may be stream references in some
        // of the atoms
        AP4_Position position;
        input->Tell(position);

        atom->Inspect(*inspector);
        input->Seek(position);
        delete atom;
    } 
    printf("%s\n", buff->GetData());
    
    if (output) output->Release();
    if (input) input->Release();
    delete inspector;

    return 0;
}
