// TODO: change to use pointers?
template <class T> struct Range {
	T first, second;

	Range() {
	}
	Range(T first, T second): first(first), second(second) {
	}

	struct iterator {
		T i;

		iterator(T i): i(i) {
		}

		T operator*() {
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
