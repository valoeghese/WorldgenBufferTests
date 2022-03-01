#include <iostream>
#include <unordered_map>
#include <stdexcept>

using std::cout;
using std::endl;

// structures

/*
typedef struct {
	long long int x;
	long long int y;
} LongVector2;
*/

struct IntVector2 {
	int x;
	int y;

	IntVector2() {
		this->x = 0;
		this->y = 0;
	}

	IntVector2(int x, int y) {
		this->x = x;
		this->y = y;
	}
};

bool operator==(const IntVector2& first, const IntVector2& second) {
	return first.x == second.x && first.y == second.y;
}

struct IntVector3 {
	int x;
	int y;
	int z;

	IntVector3() {
		this->x = 0;
		this->y = 0;
		this->z = 0;
	}

	IntVector3(int x, int y, int z) {
		this->x = x;
		this->y = y;
		this->z = z;
	}
};

std::ostream& operator<<(std::ostream& Str, const IntVector3& item) {
	Str << "(" << item.x << ", " << item.y << ", " << item.z << ")";
	return Str;
}

bool operator==(const IntVector3& first, const IntVector3& second) {
	return first.x == second.x && first.y == second.y && first.z == second.z;
}

namespace std {
	template <>
	struct hash<IntVector2> {
		std::size_t operator()(const IntVector2& k) const {
			uint64_t x = (uint32_t)k.x;
			uint64_t y = (uint32_t)k.y;
			uint64_t vec2key = x | (y << 32);

			// Call into the MSVC-STL FNV-1a std::hash function.
			return std::hash<uint64_t>()(vec2key);
		}
	};

	template <>
	struct hash<IntVector3> {
		std::size_t operator()(const IntVector3& k) const {
			uint64_t x = (uint32_t)k.x;
			uint64_t y = (uint32_t)k.y;
			uint64_t vec2key = x | (y << 32);

			uint64_t z = (uint32_t)k.z;
			vec2key = vec2key * 31L + z;

			// Call into the MSVC-STL FNV-1a std::hash function.
			return std::hash<uint64_t>()(vec2key);
		}
	};
}

// functions

int BufferArrLoc(int x_dif, int y_dif) {
	switch (y_dif) {
	case -1:
		return 1 + x_dif; // 0, 1, 2
	case 1:
		return 3 + 1 + x_dif; // 3, 4, 5
	case 0:
		return x_dif > 0 ? 6 : 7; // 6, 7
	default:
		throw std::invalid_argument("Bad Coordinates");
	}
}

typedef struct {
	std::unordered_map<IntVector3, int>* to_paste;
} ZoneBuffer;

typedef struct {
	ZoneBuffer neighbours[8];
} ZoneBufferArr8;

std::unordered_map<IntVector2, ZoneBufferArr8>* zoneBuffers;

// functions for this crap

void PasteZone(IntVector2 zone_position) {
	int base_x = zone_position.x;
	int base_y = zone_position.y;

	// search around it in a square for buffers situated in this zone
	for (int dx = -1; dx <= 1; dx++) { // MODIFIED: fix -1 to 1
		for (int dy = -1; dy <= 1; dy++) {
			if (dx == 0 && dy == 0) continue;

			IntVector2 search_location(base_x + dx, base_y + dy);

			std::unordered_map<IntVector2, ZoneBufferArr8>::iterator bufs = zoneBuffers->find(search_location);
			cout << "Looking for buffs" << endl;

			if (bufs != zoneBuffers->end()) {
				cout << "Found buffs" << endl;
				ZoneBufferArr8& buffer_collection = bufs->second;

				// reverse of dx and dy to get the relative coords of this zone from the buffer's parent zone
				int index = BufferArrLoc(-dx, -dy);
				std::unordered_map<IntVector3, int>* to_paste = buffer_collection.neighbours[index].to_paste;

				if (to_paste) {
					for (auto it = to_paste->begin(); it != to_paste->end(); it++) {
						cout << it->first << ", " <<  it->second << endl;
					}

					delete to_paste;
					buffer_collection.neighbours[index].to_paste = nullptr;
				}
			}
		}
	}
}

void SetBlockInBuffer(IntVector2 parent_pos, IntVector2 zone_pos, IntVector3 local_block_pos, int block) {
	std::unordered_map<IntVector2, ZoneBufferArr8>::iterator bufs = zoneBuffers->find(parent_pos);
	int index = BufferArrLoc(zone_pos.x - parent_pos.x, zone_pos.y - parent_pos.y);

	// I hate memory management
	if (bufs != zoneBuffers->end()) {
		ZoneBufferArr8& buffer_collection = bufs->second;

		// initialise if not yet
		if (!buffer_collection.neighbours[index].to_paste) {
			buffer_collection.neighbours[index].to_paste = new std::unordered_map<IntVector3, int>;
		}

		// add the block
		(*buffer_collection.neighbours[index].to_paste)[local_block_pos] = block;
	} else {
		// create value
		ZoneBufferArr8 new_val;
		new_val.neighbours[index].to_paste = new std::unordered_map<IntVector3, int>;
		(*new_val.neighbours[index].to_paste)[local_block_pos] = block;

		(*zoneBuffers)[parent_pos] = new_val;
	}
}

int main() {
	zoneBuffers = new std::unordered_map<IntVector2, ZoneBufferArr8>;

	cout << "Hello, World!" << endl;
	IntVector2 the_master_zone(6, 9);
	cout << "Hello, Baby Zone!" << endl;
	IntVector2 the_child_zone(6, 8);
	
	cout << "Setting Block In Buffer!" << endl;
	SetBlockInBuffer(the_master_zone, the_child_zone, IntVector3(0, 0, 0), 4);
	SetBlockInBuffer(the_master_zone, the_child_zone, IntVector3(0, 1, 0), 4);
	SetBlockInBuffer(the_master_zone, the_child_zone, IntVector3(0, 0, 1), 3);

	PasteZone(the_child_zone);
	PasteZone(the_child_zone);
}