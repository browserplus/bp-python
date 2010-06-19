import re

BrowserPlusEntryPointClass = None

# divide a list into chunks
def div_list(a, len):
    new_list = []
    for x, i in enumerate(a):
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
        ar = re.split(real_doc_string, '/^\s+([\[<])(\w+:\s*\w+)[\]>]\s+/')
        doc = ar[0]
        m = list(ar[1:])
        m = div_list(m, 3)
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
            newlist[i] = {'name': aname, 'type': atype, 'documentation': chomp2(adoc), 'required': (bracket == "<")}
            service_class.bp_doc[real_method_string] = [doc.strip(), newlist]
        return service_class
    return decorator

def bp_to_service_description(service_class):
    l = service_class.bp_doc[None]
    doc = l[0]
    init = l[1]
    hsh = {'class': service_class.name, 'name': service_class.name, 'version': service_class.bp_version, 'documentation': doc, 'functions': []}
    keys = service_class.bp_doc.keys()
    keys = keys.sort()
    flist = []
    for x, i in enumerate(keys):
        bpd = service_class.bp_doc[x]
        flist.append({'name': x, 'documentation': bpd[0], 'arguments': bpd[1]})
    hsh['functions'] = flist
    return hsh
