
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) NGINX, Inc.
 */

#ifndef _NJS_OBJECT_H_INCLUDED_
#define _NJS_OBJECT_H_INCLUDED_


#define njs_is_data_descriptor(prop)                                          \
    ((prop)->writable != NJS_ATTRIBUTE_UNSET || njs_is_valid(&(prop)->value))


#define njs_is_accessor_descriptor(prop)                                      \
    (njs_is_valid(&(prop)->getter) || njs_is_valid(&(prop)->setter))


#define njs_is_generic_descriptor(prop)                                       \
    (!njs_is_data_descriptor(prop) && !njs_is_accessor_descriptor(prop))


struct njs_object_init_s {
    njs_str_t                   name;
    const njs_object_prop_t     *properties;
    njs_uint_t                  items;
};


njs_object_t *njs_object_alloc(njs_vm_t *vm);
njs_object_t *njs_object_value_copy(njs_vm_t *vm, njs_value_t *value);
njs_object_t *njs_object_value_alloc(njs_vm_t *vm, const njs_value_t *value,
    njs_uint_t type);
njs_array_t *njs_object_enumerate(njs_vm_t *vm, const njs_object_t *object,
    njs_object_enum_t kind, njs_bool_t all);
njs_array_t *njs_object_own_enumerate(njs_vm_t *vm, const njs_object_t *object,
    njs_object_enum_t kind, njs_bool_t all);
njs_int_t njs_object_hash_create(njs_vm_t *vm, njs_lvlhsh_t *hash,
    const njs_object_prop_t *prop, njs_uint_t n);
njs_int_t njs_object_constructor(njs_vm_t *vm, njs_value_t *args,
    njs_uint_t nargs, njs_index_t unused);
njs_int_t njs_primitive_prototype_get_proto(njs_vm_t *vm, njs_value_t *value,
    njs_value_t *setval, njs_value_t *retval);
njs_int_t njs_object_prototype_create(njs_vm_t *vm, njs_value_t *value,
    njs_value_t *setval, njs_value_t *retval);
njs_value_t *njs_property_prototype_create(njs_vm_t *vm, njs_lvlhsh_t *hash,
    njs_object_t *prototype);
njs_int_t njs_object_prototype_proto(njs_vm_t *vm, njs_value_t *value,
    njs_value_t *setval, njs_value_t *retval);
njs_int_t njs_object_prototype_create_constructor(njs_vm_t *vm,
    njs_value_t *value, njs_value_t *setval, njs_value_t *retval);
njs_value_t *njs_property_constructor_create(njs_vm_t *vm, njs_lvlhsh_t *hash,
    njs_value_t *constructor);
njs_int_t njs_object_prototype_to_string(njs_vm_t *vm, njs_value_t *args,
    njs_uint_t nargs, njs_index_t unused);

njs_object_prop_t *njs_object_prop_alloc(njs_vm_t *vm, const njs_value_t *name,
    const njs_value_t *value, uint8_t attributes);
njs_object_prop_t *njs_object_property(njs_vm_t *vm, const njs_object_t *obj,
    njs_lvlhsh_query_t *lhq);
njs_int_t njs_object_prop_define(njs_vm_t *vm, njs_value_t *object,
    njs_value_t *name, njs_value_t *value);
njs_int_t njs_object_prop_descriptor(njs_vm_t *vm, njs_value_t *dest,
    njs_value_t *value, njs_value_t *setval);
const char *njs_prop_type_string(njs_object_prop_type_t type);

extern const njs_object_init_t  njs_object_constructor_init;
extern const njs_object_init_t  njs_object_prototype_init;


#endif /* _NJS_OBJECT_H_INCLUDED_ */