# Toy crypto service for testing built-in python libraries

import browserplus
import hashlib

@browserplus.bp_version("1.0.0")
@browserplus.bp_doc("A simple if digest/ built in extensions.")
@browserplus.bp_doc("sha1", "calc the sha1 of \"hello world\"")
class SHA1:
    def sha1(self, trans, args):
        m = hashlib.sha1()
        m.update("hello world")
        trans.complete(m.digest())
