/*
 * Which bundle is half-way into which target.
 *
 * A legacy bootloader keeps a partial upload across a link loss and the
 * client resumes it (see nordic-legacy-dfu, deviation 7) — correct only if
 * the bundle offered next is the one that was interrupted. Within one run
 * that is guaranteed; after a reboot of this board it is not: auto-flash
 * picks by advertised name, and the bootloader's name is what the mapping
 * sends the *application* to. So the runner notes "<address> <bundle>" in
 * a file before any SoftDevice/bootloader upload, clears it on success,
 * and on a fresh run towards the same target (or its address + 1) sends
 * that bundle regardless of the mapping — or refuses another one.
 */
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <zephyr/bluetooth/addr.h>

int  upload_memo_write(const bt_addr_le_t *addr, const char *path);
int  upload_memo_read(bt_addr_le_t *addr, char *path, size_t path_len);
void upload_memo_clear(void);
bool upload_memo_matches(const bt_addr_le_t *memo, const bt_addr_le_t *target);
