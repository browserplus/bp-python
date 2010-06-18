#
# A simple service with a bogus type in its description
#

@bp_version(BadType, "1.0.0")
@bp_doc(BadType, "A hello world test service for BrowserPlus.")
class BadType:
    @bp_doc(BadType, ":hello", "return the string, 'hello world'.  original, eh?\n\
<who: string> who to say hello to.\n\
[cb: bogus] a callback to invoke (with a bogus type name)"
    def hello(trans, args):
        args[:cb].invoke("Hi there #{args[:who]}") if args.has_key? :cb
        trans.complete("hello #{args[:who]}")
