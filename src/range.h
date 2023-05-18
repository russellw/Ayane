struct Range {
	size_t first, second;

	Range() {
	}
	Range(size_t first, size_t second): first(first), second(second) {
	}

	struct iterator {
		size_t i;

		iterator(size_t i): i(i) {
		}

		size_t operator*() {
			return i;
		}

		iterator& operator++() {
			++i;
			return *this;
		}

		bool operator!=(iterator a) {
			return i != a.i;
		}
	};

	iterator begin() {
		return first;
	}
	iterator end() {
		return second;
	}
};
