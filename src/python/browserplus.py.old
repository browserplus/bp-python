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
    def __div__(a, len):
        newl = []
        for x, i in enumerate(self):
            if (i % len == 0):
                new_list = []
                newl.append(new_list)
            sub_list = newl[i / len]
            sub_list.append(x)
            newl[i / len] = sub_list
        end
        return newl

@extend(string)
class string:
    def chomp2():
        news = self
        news = news.rstrip('\r\n')
        news = news.rstrip('\n')
        news = news.rstrip('\r')
        return news

@extend(Class)
class Class:
    def bp_version(v):
        self.bp_version = v.chomp2()

    def bp_doc(m, desc):
        global BrowserPlusEntryPointClass
        if (BrowserPlusEntryPointClass == None):
            BrowserPlusEntryPointClass = self
        elif (BrowserPlusEntryPointClass != self):
            raise RuntimeError("multiple entry point classes detected!  you may only have a single \"entry point class\" which uses bp_doc functions")
        if (self.bp_doc == None):
            self.bp_doc = {}
        if (desc == None):
            desc = m
        self.bp_doc[None] = parse_bp_doc(m, desc)

    def parse_bp_doc(mname, desc):
        ar = re.split(desc, '/^\s+([\[<])(\w+:\s*\w+)[\]>]\s+/')
        doc = ar[0]
        m = list(ar[1:])
        m = m / 3
        newlist = []
        for x, i in enumerate(m):
            bracket = x[0]
            arg = x[1]
            adoc = x[2]
            ar2 = re.split(arg, '/:\s*/')
            aname = ''
            atype = ''
            if (len(ar2) > 2):
                aname = ar2[0]
                atype = ar2[1]
            elif (len(ar2) > 1):
                aname = ar2[0]
            newlist[i] = {'name': aname, 'type': atype, 'documentation': adoc.chomp2(), 'required': (bracket == "<")}
        return [doc.strip(), newlist]

    def to_service_description():
        l = self.bp_doc[None]
        doc = l[0]
        init = l[1]
        hsh = {'class': self.name, 'name': self.name, 'version': self.bp_version, 'documentation': doc, 'functions': []}
        keys = self.bp_doc.keys()
        keys = keys.sort()
        flist = []
        for x, i in enumerate(keys):
            bpd = self.bp_doc[x]
            flist.append({'name': x, 'documentation': bpd[0], 'arguments': bpd[1]})
        hsh['functions'] = flist
        return hsh
