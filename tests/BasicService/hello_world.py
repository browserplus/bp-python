#
# A rough sketch of how a service under python engine 1.x.x may look.
# 
import browserplus
import pprint

@browserplus.bp_version("1.0.0")
@browserplus.bp_doc("A hello world test service for BrowserPlus.")
@browserplus.bp_doc("hello", "return the string, 'hello world'.  original, eh?\n\
                              <who: string> who to say hello to.\n\
                              [cb: callback] a callback to invoke")
@browserplus.bp_doc("syntax", "A function which takes no args and has a syntax error")
class HelloWorld:
    def initialize(self, args):
        print "init called!  w00t"
        pp = pprint.PrettyPrinter(index=4)
        pp.pprint(args)

    def hello(self, trans, args):
        if 'cb' in args:
            args['cb'].invoke(str.format("Hi there {0[who]}", args))
        trans.complete(str.format("hello {0[who]}", args))

    def syntax(self, trans, args):
        foo

    def destroy(self):
        print "destroy called!  thanks for calling my destructor, BrowserPlus"
