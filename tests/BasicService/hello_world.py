#
# A rough sketch of how a service under python engine 1.x.x may look.
# 
import browserplus

@browserplus.bp_version("1.0.0")
@browserplus.bp_doc("A hello world test service for BrowserPlus.")
@browserplus.bp_doc("hello", "return the string, 'hello world'.  original, eh?\n\
                              <who: string> who to say hello to.\n\
                              [cb: callback] a callback to invoke")
@browserplus.bp_doc("syntax", "A function which takes no args and has a syntax error")
class HelloWorld:
    def initialize(args):
        #require 'pp'
        print "init called!  w00t"
        #pp args

    def hello(trans, args):
        if cb in args:
            args[cb].invoke("Hi there #{args[:who]}")
        trans.complete("hello #{args[:who]}")

    def syntax(trans, args):
        foo

    def destroy():
        print "destroy called!  thanks for calling my destructor, BrowserPlus"
