
def get(valobj, name):
    value = valobj.GetChildMemberWithName(name).GetValue()
    if value is not None:
        return value

    return valobj.GetChildMemberWithName(name).GetSummary()

def get_f(valobj, name):
    value = valobj.GetChildMemberWithName(name).GetValue()
    return float(value)

def get_f_child(valobj, name, child_name):
    value = valobj.GetChildMemberWithName(name).GetChildMemberWithName(child_name).GetValue()
    return float(value)

def print_vec(valobj,internal_dict,options):
    return f"{{{get_f(valobj, 'x'):4.1f}, {get_f(valobj, 'y'):4.1f}, {get_f(valobj, 'z'):4.1f}}}"

def print_ray(valobj,internal_dict,options):
    words = [
            f"Origin: {get(valobj, 'origin')}",
            f"Direction: {get(valobj, 'direction')}",
            f"t = {get_f(valobj, 't'):3.1f}",
            f"hit = {get(valobj, 'hit')}",
            f"received = {get(valobj, 'received')}",
            f"z = {get_f_child(valobj, 'initNums', 'z')}",
            f"pre_theta = {get_f_child(valobj, 'initNums', 'pre_theta')}",
            f"i: {int(get(valobj, 'ray_start_index')):3}",

             # f"closeness = {get(valobj, 'closeness_to_sphere')}"
             ]
    col_width = max(len(word) for word in words) + 1

    return "".join(word.ljust(col_width) for word in words)
