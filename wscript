subdirs = [
    'tests'
    ]

def configure(conf):
    # conf.check_cxx(lib = 'pthread')
    conf.recurse(subdirs)


def build(bld):
    bld.recurse(subdirs)
