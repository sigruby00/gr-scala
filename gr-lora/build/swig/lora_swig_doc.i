
/*
 * This file was automatically generated using swig_doc.py.
 *
 * Any changes to it will be lost next time it is regenerated.
 */




%feature("docstring") gr::lora::decode "<+description of block+>

Constructor Specific Documentation:

Return a shared_ptr to a new instance of lora::decode.

To avoid accidental use of raw pointers, lora::decode's constructor is in a private implementation class. lora::decode::make is the public interface for creating new instances.

Args:
    spreading_factor : 
    header : 
    payload_len : 
    code_rate : 
    crc : 
    low_data_rate : "

%feature("docstring") gr::lora::decode::make "<+description of block+>

Constructor Specific Documentation:

Return a shared_ptr to a new instance of lora::decode.

To avoid accidental use of raw pointers, lora::decode's constructor is in a private implementation class. lora::decode::make is the public interface for creating new instances.

Args:
    spreading_factor : 
    header : 
    payload_len : 
    code_rate : 
    crc : 
    low_data_rate : "

%feature("docstring") gr::lora::demod "<+description of block+>

Constructor Specific Documentation:

Return a shared_ptr to a new instance of lora::demod.

To avoid accidental use of raw pointers, lora::demod's constructor is in a private implementation class. lora::demod::make is the public interface for creating new instances.

Args:
    g_id : 
    spreading_factor : 
    header : 
    payload_len : 
    cr : 
    crc : 
    low_data_rate : 
    beta : 
    fft_factor : 
    peak_search_algorithm : 
    peak_search_phase_k : 
    fs_bw_ratio : "

%feature("docstring") gr::lora::demod::make "<+description of block+>

Constructor Specific Documentation:

Return a shared_ptr to a new instance of lora::demod.

To avoid accidental use of raw pointers, lora::demod's constructor is in a private implementation class. lora::demod::make is the public interface for creating new instances.

Args:
    g_id : 
    spreading_factor : 
    header : 
    payload_len : 
    cr : 
    crc : 
    low_data_rate : 
    beta : 
    fft_factor : 
    peak_search_algorithm : 
    peak_search_phase_k : 
    fs_bw_ratio : "

%feature("docstring") gr::lora::encode "<+description of block+>

Constructor Specific Documentation:

Return a shared_ptr to a new instance of lora::encode.

To avoid accidental use of raw pointers, lora::encode's constructor is in a private implementation class. lora::encode::make is the public interface for creating new instances.

Args:
    spreading_factor : 
    code_rate : 
    crc : 
    low_data_rate : 
    header : "

%feature("docstring") gr::lora::encode::make "<+description of block+>

Constructor Specific Documentation:

Return a shared_ptr to a new instance of lora::encode.

To avoid accidental use of raw pointers, lora::encode's constructor is in a private implementation class. lora::encode::make is the public interface for creating new instances.

Args:
    spreading_factor : 
    code_rate : 
    crc : 
    low_data_rate : 
    header : "

%feature("docstring") gr::lora::mod "<+description of block+>

Constructor Specific Documentation:

Return a shared_ptr to a new instance of lora::mod.

To avoid accidental use of raw pointers, lora::mod's constructor is in a private implementation class. lora::mod::make is the public interface for creating new instances.

Args:
    spreading_factor : 
    d_sync_word : "

%feature("docstring") gr::lora::mod::make "<+description of block+>

Constructor Specific Documentation:

Return a shared_ptr to a new instance of lora::mod.

To avoid accidental use of raw pointers, lora::mod's constructor is in a private implementation class. lora::mod::make is the public interface for creating new instances.

Args:
    spreading_factor : 
    d_sync_word : "

%feature("docstring") gr::lora::pyramid_demod "<+description of block+>

Constructor Specific Documentation:

Return a shared_ptr to a new instance of lora::pyramid_demod.

To avoid accidental use of raw pointers, lora::pyramid_demod's constructor is in a private implementation class. lora::pyramid_demod::make is the public interface for creating new instances.

Args:
    g_id : 
    spreading_factor : 
    low_data_rate : 
    beta : 
    fft_factor : 
    threshold : 
    fs_bw_ratio : "

%feature("docstring") gr::lora::pyramid_demod::make "<+description of block+>

Constructor Specific Documentation:

Return a shared_ptr to a new instance of lora::pyramid_demod.

To avoid accidental use of raw pointers, lora::pyramid_demod's constructor is in a private implementation class. lora::pyramid_demod::make is the public interface for creating new instances.

Args:
    g_id : 
    spreading_factor : 
    low_data_rate : 
    beta : 
    fft_factor : 
    threshold : 
    fs_bw_ratio : "