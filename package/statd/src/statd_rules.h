#ifndef _RULES_H_
#define _RULES_H_

enum statd_destination
{
	DEST_SYSLOG,
	DEST_UNKNOWN
};

struct statd_rule
{
	char *filter;
	enum statd_destination destination;
	struct statd_rule *next;
};

/* Create rule and add to internal list */
struct statd_rule *statd_rule_add(const char* filter, enum statd_destination destination);

/* Destroy all rules */
void statd_rule_destroy_all();

/* Get first rule. Useful for looping over all rules */
struct statd_rule *statd_rule_get_head();

/* Get ubus filter for rule */
const char *statd_rule_get_filter(const struct statd_rule *rule);

/* Get log destination for rule */
enum statd_destination statd_rule_get_destination(const struct statd_rule *rule);

/* Get next rule in list */
struct statd_rule *statd_rule_get_next(const struct statd_rule *rule);

/* Returns true if current rule has a next */
int statd_rule_has_next(const struct statd_rule *rule);

#endif
