#include "defuser.h"

#include <inttypes.h>
#include <stdio.h>

#define MAX_FIELDS 8

/*
 * 把 values 中的 count 个整数写成一行“空格分隔的无符号十进制整数”，存入 out。
 * 成功返回 0；count 为 0（还没有实现）、snprintf 出错或结果被截断时返回非零。
 */
static int write_answer(char *out, size_t cap, const uint32_t *values, size_t count) {
    size_t used = 0;
    for (size_t i = 0; i < count; ++i) {
        int size = snprintf(out + used, cap - used, "%s%" PRIu32, i ? " " : "", values[i]);
        if (size < 0 || (size_t)size >= cap - used) return 1;
        used += (size_t)size;
    }
    return count == 0;
}

int solve_phase_1(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;

    // 提取 challenge 的不同部分
    uint32_t lo  = (uint32_t)challenge;
    uint32_t mid = (uint32_t)(challenge >> 16);
    uint32_t hi  = (uint32_t)(challenge >> 32);

    // 三个辅助函数
    uint32_t a = (hi + 4u * mid) ^ 0x2BCDu;
    uint32_t b = (hi + 0x8D05u) ^ lo;

    values[0] = (3u * b + a) ^ 0x6332u;

    return write_answer(out, cap, values, count);
}


int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 6;

    // 第一个整数
    values[0] = (uint32_t)((challenge >> 13) & 0x7fULL) ^ 0x1cu;

    // 后五个整数
    for (uint32_t i = 1; i < 6; i++) {
        uint32_t shift = 2 * i + 7;
        uint32_t temp = (uint32_t)((challenge >> shift) & 0x1fULL);

        values[i] = 3u * ((temp + i) ^ values[i - 1]);
    }

    return write_answer(out, cap, values, count);
}



int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 2;

    uint32_t index = (uint32_t)challenge & 7u;
    values[0] = index;

    uint32_t x = (uint32_t)(challenge >> 3)
               ^ (uint32_t)(challenge >> 32);

    switch (index) {
        case 0:
            values[1] = (x ^ 0x5ac9u) + (x >> 2);
            break;
        case 1:
            values[1] = 5u * x + 0x6a44u;
            break;
        case 2:
            values[1] = (x ^ 0x304au) + (x >> 2);
            break;
        case 3:
            values[1] = 5u * x + 0x49c14u;
            break;
        case 4:
            values[1] = (4u * x) ^ 0x7b30u;
            break;
        case 5:
            values[1] = 5u * x + 0x12ecu;
            break;
        case 6:
            values[1] = 5u * x + 0x28c26u;
            break;
        case 7:
            values[1] = (4u * x) ^ 0x1e6eu;
            break;
    }

    return write_answer(out, cap, values, count);
}



static uint32_t p4_walk_recovered(
    uint32_t target,
    uint32_t low,
    uint32_t high,
    uint32_t extra
) {
    uint32_t mid = low + ((high - low) >> 1);

    if (target == mid) {
        return (mid + 0xc2u) ^ extra;
    }

    uint32_t mix = mid + extra;

    if (target < mid) {
        uint32_t child = p4_walk_recovered(
            target, low, mid - 1, extra
        );
        return (5u * child ^ mix) ^ 0x8u;
    }

    uint32_t child = p4_walk_recovered(
        target, mid + 1, high, extra
    );
    return (5u * child ^ mix) ^ 0x1cu;
}

int solve_phase_4(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;

    uint32_t target = (uint32_t)challenge & 0x7fu;

    uint32_t extra = (uint32_t)(
        (challenge >> 7) ^ (challenge >> 32)
    );

    values[0] = p4_walk_recovered(target, 0, 127, extra);

    return write_answer(out, cap, values, count);
}



int solve_phase_5(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 2;

    typedef struct {
        int next;
        uint32_t value;
        uint32_t salt;
    } Node;

    static const Node nodes[12] = {
        { 4,  0x164u, 0xfae7u },
        { 7,  0x304u, 0x2fb0u },
        { 1,  0xa1cu, 0xf060u },
        {11,  0x86eu, 0x49fbu },
        { 8,  0x438u, 0x33c4u },
        { 9,  0x52bu, 0x881eu },
        { 2,  0x730u, 0x3480u },
        {10,  0xb6fu, 0xfa97u },
        {-1,  0x934u, 0xa8f9u },
        { 6,  0x046u, 0x8a31u },
        { 0,  0x20cu, 0x86b3u },
        { 5,  0x63eu, 0x4201u }
    };

    static const int heads[8] = {
        3, 11, 5, 9, 6, 2, 1, 7
    };

    uint32_t index = (uint32_t)(challenge & 7u);
    uint32_t steps = (uint32_t)((challenge >> 3) & 3u);

    uint32_t acc =
        ((uint32_t)(challenge >> 6)
         ^ (uint32_t)(challenge >> 32))
        ^ 0xace4u;

    int current = heads[index];
    uint32_t last_value = 0;

    for (uint32_t i = 0; i <= steps; ++i) {
        const Node *node = &nodes[current];

        last_value = node->value;
        acc = (3u * acc + node->salt) ^ node->value;

        if (i < steps) {
            current = node->next;
        }
    }

    values[0] = acc;
    values[1] = last_value;

    return write_answer(out, cap, values, count);
}



/* Phase 6: 恢复六种运算函数 */
static uint32_t p6_apply(uint32_t op, uint32_t x) {
    switch (op) {
        case 0:
            return (x ^ 0x4674u) + (x << 7);

        case 1:
            return (8u * x + 0xa8cau) ^ (7u * x);

        case 2:
            // 循环右移 7 位
            return (x >> 7) | (x << 25);

        case 3:
            return ((x >> 7) ^ x) + 0xac7bu;

        case 4:
            return (x + 0xf547u) ^ (x >> 5);

        case 5:
            // 循环左移 1 位
            return ((x << 1) | (x >> 31)) ^ 0x3c41u;

        default:
            return 0;
    }
}

int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;

    static const uint32_t ops[8] = {
        0, 0, 4, 3, 1, 3, 5, 2
    };

    uint32_t x = (uint32_t)(
        (challenge >> 6) ^ (challenge >> 32)
    );

    uint32_t index1 = (uint32_t)(challenge & 7u);
    uint32_t index2 = (uint32_t)((challenge >> 3) & 7u);

    uint32_t result = p6_apply(ops[index1], x);

    result ^= 0x2269u;

    result = p6_apply(ops[index2], result);

    values[0] = result + 0xeda3u;

    return write_answer(out, cap, values, count);
}


int solve_secret(uint64_t challenge, uint64_t context, char *out, size_t cap) {
    (void)challenge;
    (void)context;
    (void)out;
    (void)cap;
    // TODO（选做）: 根据 secret_gate、secret_copy 和 secret_phase 的汇编构造完整的第七行，写入 out 后返回 0。
    // 第七行不一定只有整数，可以直接用 snprintf 写入；还没完成时保持返回 1，测试会显示 not attempted。
    return 1;
}
