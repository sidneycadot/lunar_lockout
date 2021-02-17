#! /usr/bin/env python3

import re
import math
import subprocess

def attributes_string(attributes):
    if len(attributes) == 0:
        return ""
    return " [{}]".format("; ".join("{}=\"{}\"".format(k, v) for (k, v) in attributes.items()))

def solve(problem_nr, level, start):

    name = "problem_{:02d}".format(problem_nr)

    EMPTY = ord('.')

    vertices = set()
    edges = set()

    to_be_visited = [start]

    while to_be_visited:
        s = to_be_visited.pop()
        if s in vertices:
            continue

        vertices.add(s)

        for y in range(5):
            for x in range(5):
                mover = s[y * 5 + x]
                if mover != EMPTY:
                    for (direction, dx, dy) in [('R', +1, 0), ('U', 0, -1), ('L', -1, 0), ('D', 0, +1)]:
                        q = 1
                        while True:
                            xx = x + q * dx
                            yy = y + q * dy
                            if xx < 0 or xx > 4 or yy < 0 or yy > 4:
                                break
                            if s[yy * 5 + xx] != EMPTY:
                                if q > 1:
                                    # valid move found!
                                    xx = x + (q - 1) * dx
                                    yy = y + (q - 1) * dy
                                    ss = bytearray(s)
                                    ss[y * 5 + x] = EMPTY
                                    ss[yy * 5 + xx] = mover
                                    ss = bytes(ss)
                                    edges.add((s, ss, chr(mover) + direction))
                                    to_be_visited.append(ss)
                                break # from while loop
                            q += 1

    distance_to_solution = {}
    for v in vertices:
        if v[12] == ord('x'):
            distance_to_solution[v] = 0

    dts = 0
    while True:
        changed = False
        for (v1, v2, move_description) in edges:
            if v1 not in distance_to_solution:
                if v2 in distance_to_solution:
                    if distance_to_solution[v2] == dts:
                        distance_to_solution[v1] = dts + 1
                        changed = True
        if not changed:
            break
        dts += 1

    for v in vertices:
        if v not in distance_to_solution:
            distance_to_solution[v] = math.inf

    on_optimal_path = set()
    on_optimal_path.add(start)
    while True:
        changed = False
        for (v1, v2, move_description) in edges:
            if v1 in on_optimal_path and v2 not in on_optimal_path:
                if distance_to_solution[v2] < distance_to_solution[v1]:
                    on_optimal_path.add(v2)
                    changed = True
        if not changed:
            break

    print("name: {} vertices: {} edges: {}".format(name, len(vertices), len(edges)))

    filename_dot = "{}.dot".format(name)
    with open(filename_dot, "w") as fo:
        print("digraph {} {{".format(name), file=fo)

        attributes = {}
        attributes["shape"] = "circle"
        print("    node{};".format(attributes_string(attributes)), file=fo);

        attributes = {}
        attributes["overlap"] = "no"
        attributes["label"] = "problem {} ({})".format(problem_nr, level)
        print("    graph{};".format(attributes_string(attributes)), file=fo);
        

        dot_nv = 0
        dot_ne = 0

        for v in vertices:
            vname = v.decode().replace(".", "_")
            distance = distance_to_solution[v]

            if not math.isfinite(distance):
                continue

            label = "\\n".join(name[5 * i : 5 * (i+1)] for i in range(5)) + "\\n\\n({})".format(distance if math.isfinite(distance) else "∞")

            attributes = {}
            attributes["label"]=label
            if v == start:
                attributes["fillcolor"] = "dodgerblue"
                attributes["style"] = "filled"
            elif v[12] == ord('x') and v in on_optimal_path:
                attributes["fillcolor"] = "green"
                attributes["style"] = "filled"
            elif v[12] == ord('x'):
                attributes["fillcolor"] = "darkseagreen1"
                attributes["style"] = "filled"
            elif v in on_optimal_path:
                attributes["fillcolor"] = "cyan"
                attributes["style"] = "filled"
            elif not math.isfinite(distance):
                attributes["fillcolor"] = "red"
                attributes["style"] = "filled"
            else:
                attributes["fillcolor"] = "beige"
                attributes["style"] = "filled"

            print("    {}{};".format(vname, attributes_string(attributes)), file=fo)
            dot_nv += 1

        for (v1, v2, move_description) in edges:

            if not (math.isfinite(distance_to_solution[v1]) and math.isfinite(distance_to_solution[v2])):
                continue

            v1name = v1.decode().replace(".", "_")
            v2name = v2.decode().replace(".", "_")

            attributes = {}
            attributes["label"]=move_description

            if distance_to_solution[v2] < distance_to_solution[v1]:
                attributes["color"] = "green"

            print("    {} -> {}{};".format(v1name, v2name, attributes_string(attributes)), file=fo);
            dot_ne += 1
        print("}", file = fo)

    print("name: {} graphviz file vertices: {} edges: {}".format(name, dot_nv, dot_ne))

    filename_pdf = "{}.pdf".format(name)
    args = ["dot", "-Tpdf", filename_dot, "-o", filename_pdf]
    subprocess.run(args)

    return distance_to_solution[start]

def main():
    filename = "lunar_lockout.txt"
    re_valid = re.compile("[.abcdefghix]{5}")
    problem = []
    with open(filename, "r") as fi:
        for line in fi:
            line = line.strip()
            if line.startswith("problem"):
                (dummy, problem_nr, level) = line.split()
                problem_nr = int(problem_nr)
                assert len(problem) == 0
                continue
            match = re_valid.match(line)
            if not match:
                continue
            problem.append(line)
            if len(problem) == 5:
                s = "".join(problem).encode()
                nmoves = solve(problem_nr, level, s)
                print("problem: {} nmoves: {}".format(problem_nr, nmoves))
                problem.clear()

if __name__ == "__main__":
    main()
