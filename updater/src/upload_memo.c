#include "upload_memo.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <zephyr/fs/fs.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(upload_memo, LOG_LEVEL_INF);

#define MEMO_PATH "/lfs1/en_cours.txt"

int upload_memo_write(const bt_addr_le_t *addr, const char *path)
{
	char addr_s[BT_ADDR_LE_STR_LEN];
	char line[BT_ADDR_LE_STR_LEN + 256];
	bt_addr_le_to_str(addr, addr_s, sizeof(addr_s));
	int n = snprintf(line, sizeof(line), "%s %s\n", addr_s, path);
	if (n < 0 || (size_t)n >= sizeof(line)) {
		return -ENAMETOOLONG;
	}
	struct fs_file_t f;
	fs_file_t_init(&f);
	int rc = fs_open(&f, MEMO_PATH, FS_O_CREATE | FS_O_WRITE | FS_O_TRUNC);
	if (rc != 0) {
		return rc;
	}
	ssize_t w = fs_write(&f, line, n);
	fs_close(&f);
	if (w != n) {
		return -EIO;
	}
	LOG_INF("noted: %s is being sent to %s", path, addr_s);
	return 0;
}

int upload_memo_read(bt_addr_le_t *addr, char *path, size_t path_len)
{
	char line[BT_ADDR_LE_STR_LEN + 256];
	struct fs_file_t f;
	fs_file_t_init(&f);
	int rc = fs_open(&f, MEMO_PATH, FS_O_READ);
	if (rc != 0) {
		return -ENOENT;
	}
	ssize_t n = fs_read(&f, line, sizeof(line) - 1);
	fs_close(&f);
	if (n <= 0) {
		return -ENOENT;
	}
	line[n] = '\0';
	/* "<AA:BB:CC:DD:EE:FF> (<type>) <path>\n" */
	char addr_s[BT_ADDR_STR_LEN], type[12];
	int lus = 0;
	if (sscanf(line, "%17s (%11[^)]) %n", addr_s, type, &lus) < 2 || lus == 0) {
		return -ENOENT;
	}
	if (bt_addr_le_from_str(addr_s, type, addr) != 0) {
		return -ENOENT;
	}
	char *fin = strchr(line + lus, '\n');
	if (fin != NULL) {
		*fin = '\0';
	}
	snprintf(path, path_len, "%s", line + lus);
	return path[0] != '\0' ? 0 : -ENOENT;
}

void upload_memo_clear(void)
{
	if (fs_unlink(MEMO_PATH) == 0) {
		LOG_INF("cleared: the upload completed");
	}
}

bool upload_memo_matches(const bt_addr_le_t *memo, const bt_addr_le_t *target)
{
	if (memo->type != target->type) {
		return false;
	}
	if (bt_addr_cmp(&memo->a, &target->a) == 0) {
		return true;
	}
	/* The bootloader of a buttonless target advertises one above the
	 * application's address (least significant byte is val[0]). */
	bt_addr_t plus_un = memo->a;
	plus_un.val[0]++;
	return bt_addr_cmp(&plus_un, &target->a) == 0;
}
