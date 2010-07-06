# Toy crypto service for testing built-in ruby extensions
import browserplus
#require 'digest/sha1'

@browserplus.bp_version("1.0.0")
@browserplus.bp_doc("A simple if digest/ built in extensions.")
@browserplus.bp_doc("sha1", "calc the sha1 of \"hello world\"")
class SHA1:
    def sha1(self, trans, args):
        #trans.complete(Digest::SHA1.hexdigest("hello world"))
