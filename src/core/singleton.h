#pragma once

namespace idefix {
	template <typename V>
	class Singleton {
	public:
		static V& instance() {
			static V kInstance;
			return kInstance;
		}

	protected:
		Singleton() {}

		// Don't forget to declare these two. You want to make sure they
        // are unacceptable otherwise you may accidentally get copies of
        // your singleton appearing.
		Singleton<V>(Singleton<V> const&) = delete;
		void operator=(Singleton<V> const&) = delete;
	};
};