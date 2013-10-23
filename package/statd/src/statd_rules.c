#include "statd_rules.h"

#include <stdlib.h>
#include <string.h>

static struct statd_rule *head = NULL;
static struct statd_rule *tail = NULL;

struct statd_rule *statd_rule_add(const char* filter, enum statd_destination destination)
{
	struct statd_rule *rule = NULL;

	rule = malloc(sizeof(*rule)); 
	rule->filter = strdup(filter);
	rule->destination = destination;
	rule->next = NULL;

	if (!head) {
		head = rule;
	}

	if (tail) {
		tail->next = rule;
	}
	tail = rule;

	return rule;
}

static void statd_rule_destroy(struct statd_rule *rule)
{
	free(rule->filter);
	free(rule);
}

void statd_rule_destroy_all()
{
	struct statd_rule *current_rule = head;
	while (current_rule) {
		struct statd_rule *next = statd_rule_get_next(current_rule);
		statd_rule_destroy(current_rule);
		current_rule = next;
	}

	head = NULL;
	tail = NULL;
}

struct statd_rule *statd_rule_get_head()
{
	return head;
}

struct statd_rule *statd_rule_get_next(const struct statd_rule *rule)
{
	return rule->next;
}

const char *statd_rule_get_filter(const struct statd_rule *rule)
{
	return rule->filter;
}

enum statd_destination statd_rule_get_destination(const struct statd_rule *rule)
{
	return rule->destination;
}

int statd_rule_has_next(const struct statd_rule *rule)
{
	return rule->next != NULL;
}
