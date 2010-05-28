#
# This beautiful work was originally concieved of by why the lucky stiff:
# http://github.com/why/fakeplus/blob/master/stdlib/bp_hacks.rb
#

BrowserPlusEntryPointClass = None

# this simulates ruby's Open Classes
def extend(class_to_extend):
    def decorator(extending_class):
        class_to_extend.__dict__.update(extending_class.__dict__)
        return class_to_extend
    return decorator

@extend(list)
class list:
    def __div__(a,len):
        newa = []
        for x, i in enumerate(self):
            if (i % len == 0):
                new_list = []
                newa.append(new_list)
            sub_list = newa[i / len]
            sub_list.append(x)
            newa[i / len] = sub_list
        end
        return newa

@extend(Class)
class Class
    def bp_version(v):
        v = v.rstrip('\r\n')
        v = v.rstrip('\n')
        v = v.rstrip('\r')
        self.bp_version = v

    def bp_doc(m, desc):
        global BrowserPlusEntryPointClass
        if (BrowserPlusEntryPointClass == None):
            BrowserPlusEntryPointClass = self
        elif (BrowserPlusEntryPointClass != self):
            raise RuntimeError("multiple entry point classes detected!  you may only have a single \"entry point class\" which uses bp_doc functions")
### Incomplete port from here out ###
        self.bp_doc ||= {}
        m, desc = None, m unless desc
        self.bp_doc[m] = parse_bp_doc(m, desc)

    def parse_bp_doc(mname, desc):
        mname ||= :initialize
        ar = instance_method(mname).arity rescue -1

        doc, *m = desc.split(/^\s+([\[<])(\w+:\s*\w+)[\]>]\s+/)

        m = (m/3).map do |bracket, arg, adoc|
            aname, atype = arg.split(/:\s*/, 2)
            {'name' => aname, 'type' => atype, 'documentation' => adoc.chomp,
             'required' => bracket == "<"}
        end.compact
        [doc.strip, m]

# (lloyd) Not implementing wrapping quite yet, as it breaks async function
# returns
#
#    def bp_wrap(meth, margs):
#        alias_method "bp_#{meth}", meth
#        case meth when 'initialize'
#            define_method meth do |bp_args|
#                args = []
#                send("bp_#{meth}", *args)
#        else
#            define_method meth do |bp, bp_args|
#            begin
#                args = []
#                if (margs):
#                    args = margs.map { |a| bp_args[a['name']] }
#                bp.complete(send("bp_#{meth}", *args))
#            rescue Exception => e
#                bp.error('FAIL', "Error: #{e}")
 
    def to_service_description:
        doc, init = @bp_doc[None]
        hsh =
            {'class' => self.name,
             'name' => self.name,
             'version' => @bp_version,
             'documentation' => doc,
             'functions' => []}
        @bp_doc.each do |k, (v, m)|
            next unless k
            hsh['functions'] << {
                'name' => k.to_s,
                'documentation' => v,
                'arguments' => m
            }
        hsh
