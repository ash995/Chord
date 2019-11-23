struct chord_ip{
	char ip[16];
};
int chordInit(int my_id, char *my_ip, char *known_ip, int join);
int find_successor(int id);