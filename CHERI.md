# CHERI changes

- njs_types: NJS_64BIT was a bool macro, replace some of its usecases favor of NJS_PTR_SIZE, which can be 4, 8 or 16
- CHERI ptr provenance ambiguity fixes, eg.:
```c
#define njs_align_ptr(p, a)                                                \
    (u_char *) (((uintptr_t) (p) + ((size_t) (a) - 1))                     \
                 & ~((size_t) (a) - 1))
```

- lvlhsh:
    - `njs_lvlhsh_valid_entry` should be unified to e != 0
        - This requires to change other function signatures:
            - `njs_lvlhsh_bucket`
            - `NJS_LVLHSH_ENTRY_SIZE`: specifies the size of an entry in numbers of int32, needed to cover a pair (value, key). key is a int32 hash. This is used for pointer arithmetic when scanning for entries in a bucket. We will change this to 2 because capabilities must be aligned. This means we waste 128-32=96 bits per entry. 
            - `NJS_LVLHSH_BUCKET_END` is changed to consider every entry as `NJS_LVLHSH_ENTRY_SIZE * sizeof(uintptr_t)` 
    - `njs_lvlhsh_entry_value` is dereferenced as a pointer to `uintptr_t` instead of int32
    - `njs_lvlhsh_set_entry_value` just stores a `uintptr_t`
    - `njs_lvlhsh_entry_key` just returns the key, which is uint32
    - `njs_lvlhsh_set_entry_key` sets uint32
- `njs_int_t` is not `uintptr_t`, so in usecases to store pointers, we use `njs_ptr_t` instead, which we define as `uintptr_t`

```c
typedef struct {
    uint32_t                  bucket_end;
    uint32_t                  bucket_size;
    uint32_t                  bucket_mask;
    uint8_t                   shift[8]; // ?

    njs_lvlhsh_test_t         test;
    njs_lvlhsh_alloc_t        alloc;
    njs_lvlhsh_free_t         free;
} njs_lvlhsh_proto_t;
```

- `njs_value_t` has a different size in CHERI but it's value is hardcoded in `njs_opaque_value_t`, set to 32 bytes in CHERI
```c
gef>  p sizeof(njs_value_t)
$5 = 0x20
gef>  ptype njs_value_t
type = union njs_value_s {
    struct {
        njs_value_type_t type : 8;
        uint8_t truth;
        uint16_t magic16;
        uint32_t magic32;
        union {...} u;
    } data;
    struct {
        njs_value_type_t type : 8;
        uint8_t size : 4;
        uint8_t length : 4;
        u_char start[14];
    } short_string;
    struct {
        njs_value_type_t type : 8;
        uint8_t truth;
        uint8_t external;
        uint8_t _spare;
        uint32_t size;
        njs_string_t *data;
    } long_string;
    njs_value_type_t type : 8;
}
```

## Errors in the porting process

- CHERI Provenance due to pointer arithmetic

```
src/njs_string.c:2013:15: error: binary expression on capability types 'uintptr_t' (aka 'unsigned __intcap') and 'unsigned __intcap'; it is not clear which should be used as the source of provenance; currently provenance is inherited from the left-hand side [-Werror,-Wcheri-provenance]
        map = njs_string_map_start(end);
              ^~~~~~~~~~~~~~~~~~~~~~~~~
src/njs_string.h:40:19: note: expanded from macro 'njs_string_map_start'
    ((uint32_t *) njs_align_ptr((p), sizeof(uint32_t)))
                  ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
src/njs_clang.h:261:34: note: expanded from macro 'njs_align_ptr'
    (u_char *) (((uintptr_t) (p) + ((uintptr_t) (a) - 1))                     \
                 ~~~~~~~~~~~~~~~ ^ ~~~~~~~~~~~~~~~~~~~~~
```

- Capability tag loss due to capability-unaware memcpys:
```
Program received signal SIGPROT, CHERI protection violation.
Capability bounds fault.
njs_set_object_value (value=0xfffffff7fb10 [rwRW,0xfffffff7fb10-0xfffffff7fb20], object_value=0x41669980 [rwRW,0x41668000-0x4166a000]) at src/njs_value.h:1011
1011        value->data.u.object_value = object_value;
```
```
──────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────── trace ────
[#0] 0x18326c → njs_set_object_value(value=0x0000fffffff7fb10 [rwRW,0xfffffff7fb10-0xfffffff7fb20,len=0x10], object_value=0x0000000041669980 [rwRW,0x41668000-0x4166a000,len=0x2000])
[#1] 0x18326c → njs_vm_external_create(vm=0x0000000041670000 [rwRW,0x41670000-0x416705e0,len=0x5e0], value=0x0000fffffff7fb10 [rwRW,0xfffffff7fb10-0xfffffff7fb20,len=0x10], proto_id=0x0, external=0x0, shared=0x1)
[#2] 0x1b4a90 → njs_buffer_init(vm=0x0000000041670000 [rwRW,0x41670000-0x416705e0,len=0x5e0])
[#3] 0x156d5c → njs_vm_create(options=0x0000fffffff7fbc0 [rwRW,0xfffffff7fbc0-0xfffffff7fc50,len=0x90])
[#4] 0x14fdc0 → njs_engine_njs_init(engine=0x000000004165c000 [rwRW,0x4165c000-0x4165e000,len=0x2000], opts=0x0000fffffff7fd30 [rwRW,0xfffffff7fd30-0xfffffff7fdc0,len=0x90])
[#5] 0x14fdc0 → njs_create_engine(opts=0x0000fffffff7fd30 [rwRW,0xfffffff7fd30-0xfffffff7fdc0,len=0x90])
[#6] 0x14f618 → njs_interactive_shell(opts=<optimized out>)
[#7] 0x14f618 → njs_main(opts=<optimized out>)
[#8] 0x14f618 → main(argc=<optimized out>, argv=<optimized out>)
```

- Linking problems. The same error fires when compiling for Ubuntu 22.04. I'd say the njs fuzzer build is broken in upstream.

```
c++ -O2 -pipe -o build/njs_process_script_fuzzer  -I/usr/local/include    build/njs_process_script_fuzzer.o  build/libnjs.a  -lm   -L/usr/local/lib -Wl,-R/usr/local/lib -lpcre2-8 -lcrypto -L/usr/local/lib -lxml2 -lz
ld: error: undefined symbol: main
>>> referenced by crt1_c.c:132 (/local/scratch/jenkins/workspace/CheriBSD-pipeline_releng_24.05@2/cheribsd/lib/csu/aarch64c/crt1_c.c:132)
>>>               /usr/lib/Scrt1.o:()
>>> referenced by crt1_c.c:132 (/local/scratch/jenkins/workspace/CheriBSD-pipeline_releng_24.05@2/cheribsd/lib/csu/aarch64c/crt1_c.c:132)
>>>               /usr/lib/Scrt1.o:()
clang-14: error: linker command failed with exit code 1 (use -v to see invocation)
*** Error code 1
```
