#
# A simple BrowserPlus service implemented in Ruby that will calculate the
# MD5 sum of a file that a user has selected
#

import browserplus
#require 'digest/md5'

@browserplus.bp_version("1.0.0")
@browserplus.bp_doc("Allows client side MD5 of user selected files.")
@browserplus.bp_doc("md5", "Generate an md5 checksum of a file.\n\
                            <file: path> The file for which to calculate a checksum.")
class FileChecksum:
    def md5(self, bp, args):
        #begin
        #    contents = File.open(args[:file].realpath, "rb") { |f| f.read }
        #    bp.complete(Digest::MD5.hexdigest(contents))
        #rescue Exception => err
        #    bp.error("Error", err.to_s)
        #end
