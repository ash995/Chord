struct NODE {
	1: i64 id,
	2: string ip 
}

struct NODE_INFO {
	1: NODE me;
	2: NODE succ;
	3: NODE pred;
	4: list<NODE> f_table
}

service chordDHT {
	void notify(1:NODE node, 2:NODE pred),
	NODE get_pred(1: NODE node),
	NODE find_successor (1: i64 id)
}