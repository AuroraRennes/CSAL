#!/usr/bin/python

"""
Utility for processing CS topologies.

Reads and writes topology descriptions in various forms.
"""

from __future__ import print_function


import sys
import os
import fnmatch


import cs_topology
import cs_topology_sdf
import cs_topology_dts
import cs_topology_dot
import cs_topology_sysfs


o_verbose = 0


def load(fn):
    """
    Load a CoreSight topology description
    """
    if fn.endswith(".sdf"):
        S = cs_topology_sdf.load(fn)
    elif fn.endswith(".json"):
        S = cs_topology.load(fn)
    elif fn.endswith(".dts"):
        S = cs_topology_dts.load(fn)
    elif fn == "sysfs":
        S = cs_topology_sysfs.get_cs_from_sysfs()
    else:
        print("%s: unknown input file format" % fn, file=sys.stderr)
        sys.exit(1)
    return S


def save(S, fn):
    """
    Write out a CoreSight topology description
    """
    if fn.endswith(".sdf"):
        print("Unimplemented: can't write SDF file", file=sys.stderr)
        sys.exit(1)
    elif fn.endswith(".json"):
        print("Unimplemented: can't write JSON file", file=sys.stderr)
        sys.exit(1)
    elif fn.endswith(".dts"):
        with open(fn, "w") as f:
            cs_topology_dts.gen_dts(S, file=f)
    elif fn.endswith(".dot"):
        oo = sys.stdout
        with open(fn, "w") as f:
            sys.stdout = f
            cs_topology_dot.generate_digraph(S)
        sys.stdout = oo
    elif fn == "-":
        S.show()
    else:
        print("%s: unknown output file format" % fn, file=sys.stderr)
        sys.exit(1)


def node_matches(d, spec):
    """
    Match a node name using a case-insensitive substring.
    """
    if d.name is None:
        return False
    name = d.name.lower()
    spec = spec.lower()
    return spec in name or fnmatch.fnmatchcase(name, spec)


def matching_nodes(S, spec):
    """
    Return nodes whose names match a command-line wildcard specifier.
    """
    return [d for d in S if node_matches(d, spec)]


def atb_links(d, reverse=False):
    """
    Return ATB links from this node. If reverse is true, walk upstream.
    """
    links = d.inlinks if reverse else d.outlinks
    return [ln for ln in links if ln.linktype == cs_topology.CS_LINK_ATB]


def atb_link_next_node(ln, reverse=False):
    if reverse:
        return ln.master
    else:
        return ln.slave


def atb_link_port_label(ln):
    ports = []
    if ln.master_port is not None:
        ports.append("m%u" % ln.master_port)
    if ln.slave_port is not None:
        ports.append("s%u" % ln.slave_port)
    if ports:
        return "[%s] " % "->".join(ports)
    return ""


def show_atb_paths_from_node(d, reverse=False, indent=0, max_depth=None, path=None, port_label=""):
    """
    Print an ATB path tree starting at d. The selected node is shown at the
    left; reverse=True walks links against ATB flow.
    """
    if path is None:
        path = []
    print("%s%s%s" % (" " * (4 * indent), port_label, d))
    if max_depth is not None and indent >= max_depth:
        return
    path = path + [d]
    for ln in atb_links(d, reverse=reverse):
        nd = atb_link_next_node(ln, reverse=reverse)
        if nd in path:
            continue
        show_atb_paths_from_node(nd, reverse=reverse, indent=indent+1, max_depth=max_depth, path=path, port_label=atb_link_port_label(ln))


def show_atb_paths(S, spec, reverse=False, max_depth=None):
    nodes = matching_nodes(S, spec)
    if not nodes:
        print("No nodes match %r" % spec, file=sys.stderr)
        return 1
    for d in nodes:
        show_atb_paths_from_node(d, reverse=reverse, max_depth=max_depth)
    return 0


def main(argv):
    global o_verbose
    import argparse
    parser = argparse.ArgumentParser(description="CS topology converter")
    parser.add_argument("-i", "--input", type=str, required=True, help="input file")
    parser.add_argument("-o", "--output", type=str, action="append", default=[], help="output file(s)")
    parser.add_argument("--check", action="store_true", help="check topology")
    path_group = parser.add_mutually_exclusive_group()
    path_group.add_argument("--from", dest="from_node", type=str, help="show ATB paths from matching node name(s)")
    path_group.add_argument("--to", dest="to_node", type=str, help="show ATB paths to matching node name(s)")
    parser.add_argument("--max-depth", type=int, default=None, help="maximum ATB path depth to show")
    parser.add_argument("-v", "--verbose", action="count", default=0, help="increase verbosity")
    opts = parser.parse_args()
    if opts.max_depth is not None and opts.max_depth < 0:
        parser.error("--max-depth must be non-negative")
    o_verbose = opts.verbose
    listing_paths = opts.from_node is not None or opts.to_node is not None
    def process(fn):
        S = load(fn)
        if S is None:
            print("%s: skipping as empty" % fn, file=sys.stderr)
        else:
            if not listing_paths:
                print("%s" % S)
            if opts.check:
                res = S.check_topology()
                if res:
                    print("%s: topology issues detected" % fn, file=sys.stderr)
        return S
    if os.path.isdir(opts.input):
        # Scan directory tree looking for SDF files, and summarize/check as needed.
        if opts.output or listing_paths:
            print("Can't use output or path options when scanning directory", file=sys.stderr)
            sys.exit(1)
        for root, dirs, files in os.walk(opts.input):
            for fn in files:
                if fn.endswith(".sdf"):
                    fn = os.path.join(root, fn)
                    process(fn)
    else:
        S = process(opts.input)
        if S is not None:
            if opts.from_node is not None:
                if show_atb_paths(S, opts.from_node, max_depth=opts.max_depth):
                    sys.exit(1)
            elif opts.to_node is not None:
                if show_atb_paths(S, opts.to_node, reverse=True, max_depth=opts.max_depth):
                    sys.exit(1)
            for out in opts.output:
                save(S, out)


if __name__ == "__main__":
    main(sys.argv[1:])
