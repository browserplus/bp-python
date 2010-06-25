import re

global BrowserPlusEntryPointClass
BrowserPlusEntryPointClass = None

# divide a list into chunks
def div_list(a, len):
    new_list = []
    for i, x in enumerate(a):
        if (i % len == 0):
            new_list.append([])
        sub_list = new_list[i / len]
        sub_list.append(x)
        new_list[i / len] = sub_list
    return new_list

# python's chomp sucks
def chomp2(s):
    news = s
    news = news.rstrip('\r\n')
    news = news.rstrip('\n')
    news = news.rstrip('\r')
    return news

def bp_version(version_string):
    def decorator(service_class):
        global BrowserPlusEntryPointClass
        if (BrowserPlusEntryPointClass == None):
            BrowserPlusEntryPointClass = service_class
        elif (BrowserPlusEntryPointClass != service_class):
            raise RuntimeError("multiple entry point classes detected!  you may only have a single \"entry point class\" which uses bp_version functions")
        service_class.bp_version = chomp2(version_string)
        return service_class
    return decorator

def bp_doc(method_string, doc_string = None):
    def decorator(service_class):
        global BrowserPlusEntryPointClass
        if (BrowserPlusEntryPointClass == None):
            BrowserPlusEntryPointClass = service_class
        elif (BrowserPlusEntryPointClass != service_class):
            raise RuntimeError("multiple entry point classes detected!  you may only have a single \"entry point class\" which uses bp_doc functions")
        if (not hasattr(service_class, "bp_doc")):
            service_class.bp_doc = {}
        if (service_class.bp_doc == None):
            service_class.bp_doc = {}
        real_method_string = method_string
        real_doc_string = doc_string
        if (doc_string == None or doc_string == ""):
            real_doc_string = method_string
            real_method_string = None
        # parse it out
        rgx = re.compile(r'^\s+([\[<])(\w*:\s*\w*)[\]>]\s+', re.MULTILINE)
        ar = rgx.split(real_doc_string)
        doc = ar[0]
        m = list(ar[1:])
        m = div_list(m, 3)
        newlist = []
        if len(m) == 0:
            service_class.bp_doc[real_method_string] = [doc.strip(), newlist]
        else:
            for i, x in enumerate(m):
                bracket = x[0]
                arg = x[1]
                adoc = x[2]
                rgx2 = re.compile(r':\s*')
                ar2 = rgx2.split(arg)
                aname = ''
                atype = ''
                if (len(ar2) > 1):
                    aname = ar2[0]
                    atype = ar2[1]
                elif (len(ar2) > 0):
                    aname = ar2[0]
                newlist.append({'name': aname, 'type': atype, 'documentation': chomp2(adoc), 'required': (bracket == "<")})
                service_class.bp_doc[real_method_string] = [doc.strip(), newlist]
        return service_class
    return decorator

def bp_to_service_description(service_class):
    l = service_class.bp_doc[None]
    hsh = {'class': service_class.__name__, 'name': service_class.__name__, 'version': service_class.bp_version, 'documentation': l[0], 'functions': []}
    keys = service_class.bp_doc.keys()
    keys.sort()
    flist = []
    for i, x in enumerate(keys):
        if x == None:
            continue
        bpd = service_class.bp_doc[x]
        flist.append({'name': x, 'documentation': bpd[0], 'arguments': bpd[1]})
    hsh['functions'] = flist
    return hsh
