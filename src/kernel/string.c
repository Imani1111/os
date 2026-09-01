#include <string.h>

void kstrcpy(char* dest, const char* src){
	while (*src != '\0'){
		*dest = *src;
		dest++;
		src++;
	}
	*dest = '\0';
}

void* kmemcpy(void* dest, const void* src, uint32_t num){
	if (num == 0) return NULL;
	
	void* ret = dest;
	asm volatile (
			"rep movsb"
			: "+D" (dest), "+S" (src), "+c" (num)
			:
			: "memory"
		     );
	return ret;
}

int kstrcmp(uint8_t* str1, uint8_t* str2){
	while(*str1 && (*str1 == *str2)){
		str1++;
		str2++;
	}
	return *(const uint8_t*)str1 - *(const uint8_t*)str2;
}

void* kmemset(void* dest, uint8_t val, int count)
{
	uint8_t* ret = (uint8_t*)dest;
	for (int i = 0; i < count; i++){
		*ret = val;
		ret++;
	}
	return ret;
}
