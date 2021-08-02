const crypt = require('crypto');
const fs = require('fs');

const input = fs.createReadStream(process.argv[2]);

const algorithm = 'aes-192-cbc';
const password = 'Password used to generate key';

// First, we'll generate the key. The key length is dependent on the algorithm.
// In this case for aes192, it is 24 bytes (192 bits).
crypt.scrypt(password, 'salt', 24, (err, key) => {
  if (err) throw err;
  // Then, we'll generate a random initialization vector
  crypt.randomFill(new Uint8Array(16), (err, iv) => {
    if (err) throw err;

    // Once we have the key and iv, we can create and use the cipher...
    const cipher = crypt.createCipheriv(algorithm, key, iv);

    let encrypted = '';
    let encrypted_len = 0;
    cipher.setEncoding('hex');

    cipher.on('data', (chunk) => encrypted_len += chunk.length);
    cipher.on('end', () => console.log('encrypted:', encrypted_len));
    
    input.pipe(cipher);

    //cipher.write('some clear text data');
    //cipher.end();
  });
});
