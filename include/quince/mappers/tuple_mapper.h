#ifndef QUINCE__mappers__tuple_mapper_h
#define QUINCE__mappers__tuple_mapper_h

//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <type_traits>
#include <quince/detail/cell.h>
#include <quince/detail/compiler_specific.h>
#include <quince/detail/mapper_factory.h>
#include <quince/detail/util.h>
#include <quince/mappers/detail/abstract_mapper.h>
#include <quince/mappers/detail/exposed_mapper_type.h>
#include <quince/mapping_customization.h>


namespace quince {


//  tuple_mapper_base is for use by tuple_mapper only
//
class tuple_mapper_base : public virtual abstract_mapper_base {

    // Everything in this class is for quince internal use only.

protected:
    explicit tuple_mapper_base(const boost::optional<std::string> &name);

    virtual ~tuple_mapper_base()  {}

    template<typename Element>
    const exposed_mapper_type<Element> &
    add(const mapper_factory &creator) {
        return own(creator.create<Element>(next_created_element_name()));
    }

    template<typename ElementMapper>
    const ElementMapper &
    adopt(const ElementMapper &element_mapper) {
        if (! element_mapper.can_be_all_null())  forbid_all_null();
        return own(clone(element_mapper));
    }

private:
    std::string next_created_element_name();
    size_t _created_element_count;
};

QUINCE_SUPPRESS_MSVC_DOMINANCE_WARNING

template<typename... Es>
class basic_tuple_mapper : public abstract_mapper<std::tuple<Es...>>
{
private:
    typedef std::tuple<const exposed_mapper_type<Es> *...> member_mappers_tuple_type;

public:
    typedef std::tuple<Es...> value_type;

    // This public function is described at http://quince-lib.com/mapped_data_types/std_tuple.html
    //
    template<size_t I>
    decltype(* std::get<I>(std::declval<const member_mappers_tuple_type>()))
    get() const {
        return * std::get<I>(_member_mappers);
    }


    // --- Everything from here to end of class is for quince internal use only. ---

    basic_tuple_mapper(const boost::optional<std::string> &name, const std::tuple<const exposed_mapper_type<Es> *...>& member_mappers) :
        abstract_mapper_base(name),
        abstract_mapper<value_type>(name),
        _member_mappers(member_mappers)
    {}

    // May reintroduce this one day when more STL implementations support the corresponding feature
    // of std::tuple.
    //template<typename T>
    //const exposed_mapper_type<T> &
    //get() const {
    //    return * std::get<const exposed_mapper_type<T> *>(_member_mappers);
    //}

    template<size_t I>
    using counter_tag = std::integral_constant<size_t, I>;

    virtual void
    from_row(const row &src, value_type &dest) const override {
        from_row_helper(src, dest, counter_tag<0>());
    }

    virtual void
    to_row(const value_type &src, row &dest) const override {
        to_row_helper(src, dest, counter_tag<0>());
    }

    virtual column_id_set
    imports() const override {
        column_id_set result;
        this->imports_helper(result, counter_tag<0>());
        return result;
    }

    virtual void
    for_each_column(std::function<void(const column_mapper &)> op) const override {
        for_each_column_helper(op, counter_tag<0>());
    }

    virtual void
    for_each_persistent_column(std::function<void(const persistent_column_mapper &)> op) const override {
        for_each_persistent_column_helper(op, counter_tag<0>());
    }

    virtual void
    allow_all_null() const override {
        abstract_mapper_base::allow_all_null();
        allow_all_null_helper(counter_tag<0>());
    }

    template<typename DelayInstantiation = void>
    static void
    static_forbid_optionals() {
        static_forbid_optionals_helper(counter_tag<0>());
    }

private:
    template <typename...> friend class tuple_mapper;

    static const size_t N = sizeof...(Es);

    template<size_t I>
    void
    from_row_helper(const row &src, value_type &dest, counter_tag<I>) const {
        std::get<I>(_member_mappers)->from_row(src, std::get<I>(dest));
        from_row_helper(src, dest, counter_tag<I+1>());
    }
    void from_row_helper(const row &src, value_type &dest, counter_tag<N>) const  {}


    template<size_t I>
    void
    to_row_helper(const value_type &src, row &dest, counter_tag<I>) const {
        std::get<I>(_member_mappers)->to_row(std::get<I>(src), dest);
        to_row_helper(src, dest, counter_tag<I+1>());
    }
    void to_row_helper(const value_type &src, row &dest, counter_tag<N>) const  {}


    template<size_t I>
    void
    imports_helper(column_id_set &accumulator, counter_tag<I>) const {
        add_to_set(accumulator, std::get<I>(_member_mappers)->imports());
        imports_helper(accumulator, counter_tag<I+1>());
    }
    void imports_helper(column_id_set &, counter_tag<N>) const  {}


    template<size_t I>
    void
    for_each_column_helper(std::function<void(const column_mapper &)> op, counter_tag<I>) const {
        std::get<I>(_member_mappers)->for_each_column(op);
        for_each_column_helper(op, counter_tag<I+1>());
    }
    void for_each_column_helper(std::function<void(const column_mapper &)>, counter_tag<N>) const  {}


    template<size_t I>
    void
    for_each_persistent_column_helper(std::function<void(const persistent_column_mapper &)> op, counter_tag<I>) const {
        std::get<I>(_member_mappers)->for_each_persistent_column(op);
        for_each_persistent_column_helper(op, counter_tag<I+1>());
    }
    void for_each_persistent_column_helper(std::function<void(const persistent_column_mapper &)>, counter_tag<N>) const  {}


    template<size_t I>
    void
    allow_all_null_helper(counter_tag<I>) const {
        std::get<I>(_member_mappers)->allow_all_null();
        allow_all_null_helper(counter_tag<I+1>());
    }
    void allow_all_null_helper(counter_tag<N>) const  {}

    template<size_t I>
    static void
    static_forbid_optionals_helper(counter_tag<I>) {
        typedef typename std::remove_pointer<typename std::tuple_element<I, member_mappers_tuple_type>::type>::type member_mapper_type;
        member_mapper_type::static_forbid_optionals();
        static_forbid_optionals_helper(counter_tag<I+1>());
    }
    static void static_forbid_optionals_helper(counter_tag<N>)  {}

    const member_mappers_tuple_type _member_mappers;
};

// tuple_mapper<Es...> is the mapper class for std::tuple<Es...>
//
template<typename... Es>
class tuple_mapper : public tuple_mapper_base, public basic_tuple_mapper<Es...>
{
public:
    // --- Everything from here to end of class is for quince internal use only. ---

    // Constructor that is used whenever *this is to be some table's value mapper, or part of some table's
    // value mapper:
    //
    tuple_mapper(const boost::optional<std::string> &name, const mapper_factory &creator) :
        abstract_mapper_base(name),
        tuple_mapper_base(name),

        // I want to say:
        //
        //      basic_tuple_mapper<Es...>{name, &add<Es>(creator)...}
        //
        // The language spec says that elements of a braced initializer list are evaluated
        // left-to-right, so the add()s should generate their sequential column names properly,
        // i.e. in ascending order from left to right.  However MSVC doesn't respect that
        // language rule, so what follows is a workaround.
        //
        // TODO: when MSVC is fixed, do what I wanted and then delete make_member_mappers()
        // and its helper.  Probably also change the elements of _member_mappers, from pointers
        // to references.
        //
        basic_tuple_mapper<Es...>(name, make_member_mappers(creator))
    {}

    // Constructor that is used whenever *this is to be a collector class for select() or a function in the join() family:
    //
    explicit tuple_mapper(const exposed_mapper_type<Es> &... member_mappers) :
        abstract_mapper_base(boost::none),
        tuple_mapper_base(boost::none),
        basic_tuple_mapper<Es...>(boost::none, std::make_tuple(&adopt(member_mappers)...))
    {}

    virtual std::unique_ptr<cloneable>
    clone_impl() const override {
        return quince::make_unique<tuple_mapper<Es...>>(*this);
    }

private:
    template<size_t I>
    using counter_tag = std::integral_constant<size_t, I>;

    static const size_t N = sizeof...(Es);

    std::tuple<const exposed_mapper_type<Es> * ...>
    make_member_mappers(const mapper_factory &creator) {
        std::tuple<const exposed_mapper_type<Es> * ...> result;
        make_member_mappers_helper(creator, result, counter_tag<0>());
        return result;
    }

    template<size_t I>
    void
    make_member_mappers_helper(
        const mapper_factory &creator,
        std::tuple<const exposed_mapper_type<Es>*...> &accumulator,
        counter_tag<I>
    ) {
        auto &target = std::get<I>(accumulator);
        typedef decltype(target->value_declval()) value_type;
        target = &add<value_type>(creator);
        make_member_mappers_helper(creator, accumulator, counter_tag<I+1>());
    }
    void make_member_mappers_helper(const mapper_factory &, std::tuple<const exposed_mapper_type<Es>*...> &, counter_tag<N>)  {}
};

template<typename... Es>
class tuple_mapper_ref : public basic_tuple_mapper<Es...>
{
public:
    explicit tuple_mapper_ref(const exposed_mapper_type<Es> &... member_mappers) :
        abstract_mapper_base(boost::none),
        basic_tuple_mapper<Es...>(boost::none, std::make_tuple(&member_mappers...))
    {}

    virtual std::unique_ptr<cloneable>
    clone_impl() const override {
        return quince::make_unique<tuple_mapper_ref<Es...>>(*this);
    }
};

template <typename... Es> tuple_mapper_ref(const abstract_mapper<Es> &...) -> tuple_mapper_ref<Es...>;

QUINCE_UNSUPPRESS_MSVC_WARNING

}

#endif
