ENTRY(_start)

SECTIONS
{
    .text 0x1000 :
    {
        *(.text)
    }
    .data :AT(ADDR(.text)+SIZEOF(.text))
    {
        *(.data)
    }
}
