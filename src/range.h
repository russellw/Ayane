struct Range: pair<size_t, size_t> {
	struct iterator {
		size_t i;

		iterator(size_t i): i(i) {}

		size_t operator*() { return i; }

		iterator& operator++() {
			++i;
			return *this;
		}

		bool operator!=(iterator a) { return i != a.i; }
	};

	Range() {}
	Range(size_t first, size_t second): pair(first, second) {}

	iterator begin() { return first; }
	iterator end() { return second; }
};
