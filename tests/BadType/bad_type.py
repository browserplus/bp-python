#
# A simple service with a bogus type in its description
#
import browserplus

@browserplus.bp_version("1.0.0")
@browserplus.bp_doc("A hello world test service for BrowserPlus.")
@browserplus.bp_doc("hello:", "return the string, 'hello world'.  original, eh?\n\
                               <who: string> who to say hello to.\n\
                               [cb: bogus] a callback to invoke (with a bogus type name)")
class BadType:
    def hello(trans, args):
        if cb in args:
            args[cb].invoke("Hi there #{args[:who]}")
        trans.complete("hello #{args[:who]}")
