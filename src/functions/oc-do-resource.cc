#include "oc-do-resource.h"
#include "../functions-internal.h"

extern "C" {
#include <stdlib.h>
#include <ocstack.h>
}

using namespace v8;

static OCStackApplicationResult defaultOCClientResponseHandler(
		void *context, OCDoHandle handle, OCClientResponse *clientResponse ) {
	Isolate *isolate = Isolate::GetCurrent();
	Local<Function> jsCallback = Local<Function>::New( isolate, *( Persistent<Function> * )context );
	
}

static void defaultOCClientContextDeleter( void *context ) {
	delete ( Persistent<Object> * )context;
}

static OCHeaderOption *oc_header_options_new( Isolate *isolate, Handle<Array> array ) {
	int index, optionIndex, optionLength;
	int count = array->Length();
	OCHeaderOption *options = ( OCHeaderOption * )malloc( sizeof( OCHeaderOption ) * count );

	if ( options ) {
		for ( index = 0 ; index < count ; index++ ) {
			options[ index ].protocolID =
				( OCTransportProtocolID )array->Get( index )->ToObject()
					->Get( String::NewFromUtf8( isolate, "protocolID" ) )->Uint32Value();
			options[ index ].optionID =
				( uint16_t )array->Get( index )->ToObject()
					->Get( String::NewFromUtf8( isolate, "optionID" ) )->Uint32Value();
			options[ index ].optionLength =
				( uint16_t )array->Get( index )->ToObject()
					->Get( String::NewFromUtf8( isolate, "optionLength" ) )->Uint32Value();

			Handle<Array> jsOption = Handle<Array>::Cast(
				array->Get( index )->ToObject()
					->Get( String::NewFromUtf8( isolate, "optionData" ) ) ) ;
			optionLength = jsOption->Length();
			optionLength = ( optionLength > MAX_HEADER_OPTION_DATA_LENGTH ) ?
				MAX_HEADER_OPTION_DATA_LENGTH : optionLength;

			for ( optionIndex = 0 ; optionIndex < optionLength ; optionIndex++ ) {
				options[ index ].optionData[ optionIndex ] =
					( uint8_t )jsOption->Get( optionIndex )->Uint32Value();
			}
		}
	}

	return options;
}

// Always returns NULL
static OCHeaderOption *oc_header_options_free( OCHeaderOption *options ) {
	free( ( void * )options );

	return 0;
}

void bind_OCDoResource( const FunctionCallbackInfo<Value>& args ) {
	Isolate *isolate = Isolate::GetCurrent();
	OCHeaderOption *options = 0;
	OCDoHandle handle;
	OCCallbackData data = {
		0,
		defaultOCClientResponseHandler,
		defaultOCClientContextDeleter
	};

	VALIDATE_ARGUMENT_COUNT( isolate, args, 9 );
	VALIDATE_ARGUMENT_TYPE( isolate, args, 0, IsObject );
	VALIDATE_ARGUMENT_TYPE( isolate, args, 1, IsUint32 );
	VALIDATE_ARGUMENT_TYPE( isolate, args, 2, IsString );
	VALIDATE_ARGUMENT_TYPE( isolate, args, 3, IsString );
	VALIDATE_ARGUMENT_TYPE( isolate, args, 4, IsString );
	VALIDATE_ARGUMENT_TYPE( isolate, args, 5, IsUint32 );
	VALIDATE_ARGUMENT_TYPE( isolate, args, 6, IsFunction );
	VALIDATE_ARGUMENT_TYPE( isolate, args, 7, IsArray );
	VALIDATE_ARGUMENT_TYPE( isolate, args, 8, IsUint32 );

	data.context = ( void * )new Persistent<Function>( isolate, Local<Function>::Cast( args[ 7 ] ) );

	options = oc_header_options_new( isolate, Handle<Array>::Cast( args[ 0 ] ) );

	args.GetReturnValue().Set( Number::New( isolate,
		( double )OCDoResource(
			&handle,
			( OCMethod )args[ 1 ]->Uint32Value(),
			( const char * )*String::Utf8Value( args[ 2 ] ),
			( const char * )*String::Utf8Value( args[ 3 ] ),
			( const char * )*String::Utf8Value( args[ 4 ] ),
			( OCQualityOfService )args[ 5 ]->Uint32Value(),
			&data,
			options,
			( uint8_t )args[ 8 ]->Uint32Value() ) ) );

	options = oc_header_options_free( options );
}