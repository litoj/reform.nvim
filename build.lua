os.execute('cd "' .. debug.getinfo(1, 'S').source:sub(2, -10) .. '" && make &>/dev/null')
