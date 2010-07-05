# Toy crypto service for testing built-in python libraries
#

import browserplus
import ssl
#import os
#from base64 import b64encode
#from M2Crypto import RSA
#import md5
#import RSA
import hashlib

@browserplus.bp_version("1.0.0")
@browserplus.bp_doc("A tool for generating RSA keypairs on the client.")
@browserplus.bp_doc("listKeys", "list public keys associated with the user on this domain")
@browserplus.bp_doc("generate", "generate a new RSA public keypair, returning a base64 \
                                encoded representation of the public keys")
class Crypto:
    def initialize(self, args):
        self.keys = []

    def listKeys(self, trans, args):
        # For this test, we do not persist keys.
        #trans.complete(@keys.collect {|x| [x.public_key.to_s].pack('m')})
        print "A"

    def generate(self, trans, args):
        # Generate an RSA key.
        #rsa = OpenSSL::PKey::RSA.generate(1024)
        #self.keys.push(rsa)
        # For this test, we do not persist keys.
        #trans.complete([rsa.public_key.to_s].pack('m'))
        print "A"

    def destroy(self):
        print "destroy called!  thanks for calling my destructor, BrowserPlus"
