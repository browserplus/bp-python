#
# A service that requires another file
#
import browserplus
import myinclude

@browserplus.bp_version("1.0.0")
@browserplus.bp_doc("A hello world test service for BrowserPlus.")
@browserplus.bp_doc("yo", "return a string")
class HelloWorld:
    def yo(self, trans, args):
        trans.complete(myinclude.getAString())
