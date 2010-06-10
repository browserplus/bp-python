# divide a list into chunks
def div_list(a, len):
    new_list = []
    for x, i in enumerate(a):
        if (i % len == 0):
            new_list.append([])
        sub_list = new_list[i / len]
        sub_list.append(x)
        new_list[i / len] = sub_list
    end
    return new_list

# python's chomp sucks
def chomp2(s):
    news = s
    news = news.rstrip('\r\n')
    news = news.rstrip('\n')
    news = news.rstrip('\r')
    return news

BrowserPlusEntryPointClass = None

def bp_version(service_class, version_string):
    service_class.bp_version = chomp2(version_string)

def bp_doc(service_class, service_method, doc_string):
    global BrowserPlusEntryPointClass
    if (BrowserPlusEntryPointClass == None):
        BrowserPlusEntryPointClass = service_class
    elif (BrowserPlusEntryPointClass != service_class):
        raise RuntimeError("multiple entry point classes detected!  you may only have a single \"entry point class\" which uses bp_doc functions")
    if (service_class.bp_doc == None):
        service_class.bp_doc = {}
    if (desc == None):
        desc = m
    # parse it out
    ar = re.split(desc, '/^\s+([\[<])(\w+:\s*\w+)[\]>]\s+/')
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
    service_class.bp_doc[None] = [doc.strip(), newlist]

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
