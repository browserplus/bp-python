# a rough sketch of how a service under ruby engine 5.x.x may look,
# 
# we'll need some of why's goodness to get here:
# http://github.com/why/fakeplus/blob/master/stdlib/bp_hacks.rb
#
# design credit? why, the lucky stiff, of course.
#

class HelloWorld
  bp_version "1.0.0"
  bp_doc "A hello world test service for BrowserPlus."

  def hello(trans, args)
    args[:cb].invoke("Hi there #{args[:who]}") if args.has_key? :cb
    trans.complete("hello #{args[:who]}")
  end

  bp_doc :hello, "return the string, 'hello world'.  original, eh?
                  <who: string> who to say hello to.
                  [cb: callback] a callback to invoke"
end

class Foo
  bp_version "1.0.0"
  bp_doc "A hello world test service for BrowserPlus."

  def hello(trans, args)
    args[:cb].invoke("Hi there #{args[:who]}") if args.has_key? :cb
    trans.complete("hello #{args[:who]}")
  end

  bp_doc :hello, "return the string, 'hello world'.  original, eh?
                  <who: string> who to say hello to.
                  [cb: callback] a callback to invoke"
end

