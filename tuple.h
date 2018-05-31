#pragma once
#include <memory>
#include <type_traits>
template <class T>
struct unwrap_refwrapper
{
	using type = T;
};

template <class T>
struct unwrap_refwrapper<std::reference_wrapper<T>>
{
	using type = T&;
};

template <class T>
using special_decay_t = typename unwrap_refwrapper<typename std::decay<T>::type>::type;

template <typename ...Types>
class Tuple;

template <typename T, typename ...Types>
class Tuple<T, Types...> {
public:
	T el_;
	Tuple <Types...> right_;
	T& el() & {
		return el_;
	}
	T&& el() && {
		return std::move(el_);
	}
	const T& el() const & {
		return el_;
	}

	Tuple() : el_(), right_() {}

	template <typename ...all_U>
	explicit Tuple(std::add_rvalue_reference_t<std::decay_t <T>> el_, all_U &&...args) : 
		el_(std::move(el_)),
		right_(std::forward <all_U>(args)...) 
	{}
	template <typename ...all_U>
	explicit Tuple(std::add_lvalue_reference_t<std::add_const_t<std::decay_t <T>>> el_, all_U &&...args) :
		el_(el_),
		right_(std::forward <all_U>(args)...)
	{}
	template <typename ...all_U>
	explicit Tuple(std::add_lvalue_reference_t<std::decay_t <T>> el_, all_U &&...args) :
		el_(el_),
		right_(std::forward <all_U>(args)...)
	{}

	explicit Tuple(std::add_rvalue_reference_t<std::decay_t <T>> el_) : el_(std::move(el_)) {}
	explicit Tuple(std::add_lvalue_reference_t<std::add_const_t<std::decay_t <T>>> el_) : el_(el_) {}
	explicit Tuple(std::add_lvalue_reference_t<std::decay_t <T>> el_) : el_(el_) {}

	Tuple(const Tuple &old) : right_(old.right_), el_(old.el_) {}
	Tuple(Tuple &&old) : right_(std::forward<decltype(old.right_)>(old.right_)), el_(std::forward<decltype(old.el_)>(old.el_)) {}
	~Tuple() {}

	Tuple &operator=(Tuple &&other) {
		right_ = std::forward<decltype(other.right_)>(other.right_);
		el_ = std::forward<decltype(other.el_)>(other.el_);
		return *this;
	}
	Tuple &operator=(const Tuple &other) {
		right_ = other.right_;
		el_ =  other.el_;
		return *this;
	}
	void swap(Tuple &other) {
		std::swap(el_, other.el_);
		std::swap(right_, other.right_);
	}
	bool operator == (const Tuple &other) const {
		return (el_ == other.el_) && (other.right_ == right_);
	}
	bool operator != (const Tuple &other) const {
		return (!(*this) == other);
	}
	bool operator < (const Tuple &other) const {
		return (el_ < other.el_) ||(other.right_ < right_);
	}
	bool operator <= (const Tuple &other) const {
		return ((*this) < other) || ((*this) == other);
	}
	bool operator > (const Tuple &other) const {
		return other < (*this);
	}
	bool operator >= (const Tuple &other) const {
		return !((*this) < other);
	}
};
template <> 
class Tuple <> { 
public:
	bool operator == (const Tuple &other) const {
		return true;
	}
	bool operator != (const Tuple &other) const {
		return false;
	}
	bool operator < (const Tuple &other) const {
		return false;
	}
	bool operator <= (const Tuple &other) const {
		return true;
	}
	bool operator > (const Tuple &other) const {
		return false;
	}
	bool operator >= (const Tuple &other) const {
		return true;
	}
};



template <size_t index, typename...Args, typename std::enable_if_t <(index == 0), int> = 0>
decltype(auto) get(Tuple<Args...> &&t) {
	return std::move(t.el());
}
template <size_t index, typename...Args, typename std::enable_if_t <(index > 0), int> = 0>
decltype(auto) get(Tuple<Args...> &&t) {
	return /*static_cast <std::add_rvalue_reference_t <std::decay_t <decltype(right_.get_i<index - 1>())>>>*/
		std::move(get<index - 1>(std::move(t.right_)));
}

template <size_t index, typename...Args, typename std::enable_if_t <(index == 0), int> = 0>
decltype(auto) get(Tuple<Args...> &t) {
	return t.el();
}
template <size_t index, typename...Args, typename std::enable_if_t <(index > 0), int> = 0>
decltype(auto) get(Tuple<Args...> &t) {
	return /*static_cast <std::add_rvalue_reference_t <std::decay_t <decltype(right_.get_i<index - 1>())>>>*/
		get<index - 1>(t.right_);
}

template <size_t index, typename...Args, typename std::enable_if_t <(index == 0), int> = 0>
decltype(auto) get(const Tuple<Args...> &t) {
	return t.el();
}
template <size_t index, typename...Args, typename std::enable_if_t <(index > 0), int> = 0>
decltype(auto) get(const Tuple<Args...> &&t) {
	return /*static_cast <std::add_rvalue_reference_t <std::decay_t <decltype(right_.get_i<index - 1>())>>>*/
		get<index - 1>(t.right_);
}


template <typename U, typename T, typename...Args, typename std::enable_if_t <std::is_same <T, U>::value, int> = 0>
decltype(auto) get(Tuple <T, Args...> &&t) {
	return std::move(t.el());
}
template <typename U, typename T, typename...Args, typename std::enable_if_t <!std::is_same <T, U>::value, int> = 0>
decltype(auto) get(Tuple <T, Args...> &&t) {
	return /*static_cast <std::add_rvalue_reference_t <std::decay_t <decltype(right_.get_same<U>())>>>*/
		std::move(get<U, Args...>(std::move(t.right_)));
}

template <typename U, typename T, typename...Args, typename std::enable_if_t <std::is_same <T, U>::value, int> = 0>
decltype(auto) get(Tuple <T, Args...> &t) {
	return t.el();
}
template <typename U, typename T, typename...Args, typename std::enable_if_t <!std::is_same <T, U>::value, int> = 0>
decltype(auto) get(Tuple <T, Args...> &t) {
	return /*static_cast <std::add_rvalue_reference_t <std::decay_t <decltype(right_.get_same<U>())>>>*/
		get<U, Args...>(t.right_);
}

template <typename U, typename T, typename...Args, typename std::enable_if_t <std::is_same <T, U>::value, int> = 0>
decltype(auto) get(const Tuple <T, Args...> &t) {
	return t.el();
}
template <typename U, typename T, typename...Args, typename std::enable_if_t <!std::is_same <T, U>::value, int> = 0>
decltype(auto) get(const Tuple <T, Args...> &t) {
	return /*static_cast <std::add_rvalue_reference_t <std::decay_t <decltype(right_.get_same<U>())>>>*/
		get<U, Args...>(t.right_);
}


template <class... Types>
auto makeTuple(Types&&... args)
{
	return Tuple<special_decay_t<Types>...>(std::forward<Types>(args)...);
}


template <typename T1, typename T2>
struct union_tuple;

template <typename...T1, typename...T2>
struct union_tuple <Tuple<T1...>, Tuple<T2...>> {
	typedef Tuple <T1..., T2...> type;
};

template <class L, class R, class Arg1, class... Args>
decltype(auto) tupleCat(L &&left, R &&right, Arg1 &&back1, Args &&...back) {
	return tupleCat(tupleCat(std::forward<L>(left), std::forward<R>(right)), std::forward <Arg1>(back1), std::forward <Args>(back)...);
};

template <typename T, typename... L, class R>
decltype(auto) tupleCat(const Tuple <T, L...> &left, R &&right) {
	return typename union_tuple<Tuple<T, L...>, std::decay_t<R>>::type(left.el_, tupleCat(left.right_, std::forward <R>(right)));
};
template <typename T, typename... L, class R>
decltype(auto) tupleCat(Tuple <T, L...> &&left, R &&right) {
	return typename union_tuple<Tuple<T, L...>, std::decay_t<R>>::type(std::forward<T>(left.el_), tupleCat(std::move(left.right_), std::forward <R>(right)));
};

template <class R>
decltype(auto) tupleCat(const Tuple <> &, R &&right) {
	return std::forward<R>(right);
}
