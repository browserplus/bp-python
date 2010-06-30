#
# A simple service with a bogus type in its description
#
import browserplus

@browserplus.bp_version("1.0.0")
@browserplus.bp_doc("A hello world test service for BrowserPlus.")
@browserplus.bp_doc("hello", "return the string, 'hello world'.  original, eh?\n\
                              <who: string> who to say hello to.\n\
                              [cb: bogus] a callback to invoke (with a bogus type name)")
class BadType:
    def hello(self, trans, args):
        if 'cb' in args:
            args['cb'].invoke(str.format("Hi there {0[who]}", args))
        trans.complete(str.format("hello {0[who]}", args))
