#
# A simple BrowserPlus service implemented in Python that will calculate the
# MD5 sum of a file that a user has selected
#

import browserplus
import hashlib

@browserplus.bp_version("1.0.0")
@browserplus.bp_doc("Allows client side MD5 of user selected files.")
@browserplus.bp_doc("md5", "Generate an md5 checksum of a file.\n\
                            <file: path> The file for which to calculate a checksum.")
class FileChecksum:
    def md5(self, bp, args):
        try:
            contents = ""
            if 'file' in args:
                f = open(args['file'].realpath, "rb")
                try:
                    contents = f.read()
                finally:
                    f.close()
            m = hashlib.md5()
            m.update(contents)
            bp.complete(m.digest())
        except Exception as err:
            bp.error("Error", err.__str__())
